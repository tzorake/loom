# cmake/LoomHelpers.cmake
#
# Provides loom_add_executable(<target> <sources...>) — a single-call helper
# that creates a Loom application target for any supported platform.
#
# ── How it works ─────────────────────────────────────────────────────────────
#
# For native builds (macOS / Windows / Linux):
#   • Creates a regular executable linked against loom-widgets.
#   • Auto-generates a thin main() trampoline that forwards to loom_main().
#
# For WebAssembly (WASI) builds:
#   • Creates a .wasm module linked against loom-widgets.
#   • Auto-generates a loom_init() export that forwards to loom_main().
#   • Copies index.html and loom.js alongside the .wasm so the build output
#     directory is self-contained and can be served directly:
#       python3 -m http.server 8080 --directory <build>/path/to/target
#
# ── Usage ─────────────────────────────────────────────────────────────────────
#
#   # CMakeLists.txt
#   include(cmake/LoomHelpers.cmake)          # or via find_package(Loom)
#
#   loom_add_executable(my_app my_app.cpp)
#
#   # Add extra libraries if needed:
#   target_link_libraries(my_app PRIVATE loom-models)
#
# ── Entry point convention ────────────────────────────────────────────────────
#
#   Your source file must define:
#       int loom_main(int argc, char *argv[]);
#
#   Do NOT define main() — it is generated automatically by this helper.
#
# ── Web compatibility note ────────────────────────────────────────────────────
#
#   On WebAssembly, app.exec() returns immediately (the browser's
#   requestAnimationFrame drives the event loop instead of blocking).
#   This means any TzGuiApplication or TzWindow objects created inside
#   loom_main() must be heap-allocated (via new) so they survive loom_main()
#   returning.  On native, heap- or stack-allocation both work correctly
#   because exec() blocks until the application exits.
#
#   Example pattern that works on both platforms:
#
#       int loom_main(int argc, char *argv[])
#       {
#           // Heap-allocate for web compat; leaks on native exit (benign).
#           auto *app    = new TzGuiApplication(argc, argv);
#           auto *window = new MyWindow();
#           window->setTitle("My App");
#           window->show();
#           return app->exec();
#       }
#
# ── Web assets ────────────────────────────────────────────────────────────────
#
#   LOOM_WEB_DIR must point to the directory containing index.html and loom.js.
#   It defaults to <this_file>/../web, which is correct when building loom
#   itself.  External projects that consume loom should set LOOM_WEB_DIR to
#   the installed web/ directory before calling loom_add_executable().

cmake_minimum_required(VERSION 3.25)

# Directory containing this file — used to locate sibling cmake scripts.
set(_LOOM_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

# Default web assets directory: lives with the rest of the web platform code.
# Explicitly unset any stale cache entry so changing this path never requires
# a manual cache wipe.  Override in your own CMakeLists.txt (before including
# this file) if your layout differs.
unset(LOOM_WEB_DIR CACHE)
if(NOT DEFINED LOOM_WEB_DIR)
    set(LOOM_WEB_DIR "${CMAKE_CURRENT_LIST_DIR}/../src/core/src/platform/web")
endif()

# ── loom_add_executable ───────────────────────────────────────────────────────

function(loom_add_executable target)
    set(_sources ${ARGN})

    if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
        # ── Web (WASM) ────────────────────────────────────────────────────────

        add_executable(${target} ${_sources} "${_LOOM_WEB_INIT_SRC}")
        target_link_libraries(${target} PRIVATE loom-widgets)
        set_target_properties(${target} PROPERTIES
            OUTPUT_NAME "loom_app"
            SUFFIX      ".wasm"
            # Each target gets its own subdirectory so multiple WASM apps
            # in the same build tree don't overwrite each other's loom_app.wasm.
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${target}"
        )

        # Remove any stale output before linking so the linker always writes a
        # fresh file.  Without this, CMake's clean-first or interrupted builds
        # can leave a zero-filled or truncated .wasm that the linker maps and
        # overwrites in-place, producing a corrupt output on some filesystems.
        add_custom_command(TARGET ${target} PRE_LINK
            COMMAND ${CMAKE_COMMAND} -E remove -f
                "${CMAKE_CURRENT_BINARY_DIR}/${target}/loom_app.wasm"
            COMMENT "Removing stale ${target}/loom_app.wasm before link"
        )

        # Force the OS to flush the output file to physical storage before any
        # further steps read or serve it.  On external/network drives the kernel
        # write-back cache can lag behind, leaving a zero-filled file on disk
        # even though in-memory reads look valid.
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DWASM_FILE="$<TARGET_FILE:${target}>"
                -P "${_LOOM_CMAKE_DIR}/SyncWasm.cmake"
            COMMENT "Flushing ${target}/loom_app.wasm to disk"
        )

        # Verify the linker produced a valid WASM binary (magic bytes \0asm).
        # Fails loudly if the output is zero-filled or truncated so a corrupt
        # file is never silently deployed.
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND}
                -DWASM_FILE="$<TARGET_FILE:${target}>"
                -P "${_LOOM_CMAKE_DIR}/CheckWasm.cmake"
            COMMENT "Verifying WASM magic bytes in $<TARGET_FILE_NAME:${target}>"
        )

        # Copy web assets alongside the .wasm output so the directory is
        # self-contained.  copy_if_different avoids unnecessary rebuilds.
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LOOM_WEB_DIR}/index.html"
                "$<TARGET_FILE_DIR:${target}>/index.html"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${LOOM_WEB_DIR}/loom.js"
                "$<TARGET_FILE_DIR:${target}>/loom.js"
            COMMENT "Copying web assets → $<TARGET_FILE_DIR:${target}>"
        )

    else()
        # ── Native (macOS / Windows / Linux) ──────────────────────────────────

        add_executable(${target} ${_sources} "${_LOOM_NATIVE_MAIN_SRC}")
        target_link_libraries(${target} PRIVATE loom-widgets)
    endif()
endfunction()

# ── Shared native trampoline (generated once at include time) ─────────────────
#
# All native targets share one _loom_native_main.cpp.  Writing it at module
# scope (not inside the function) means the file is created once per
# CMakeLists.txt include, not once per loom_add_executable() call.
# file(CONFIGURE) only rewrites the file when content changes, so it won't
# trigger unnecessary recompiles on reconfigure.

if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
    set(_LOOM_WEB_INIT_SRC "${CMAKE_BINARY_DIR}/_loom_web_init.cpp")
    file(CONFIGURE OUTPUT "${_LOOM_WEB_INIT_SRC}" CONTENT
"// Auto-generated by LoomHelpers.cmake — do not edit.\n\
extern int loom_main(int argc, char *argv[]);\n\
static int   s_argc    = 0;\n\
static char *s_argv[1] = {nullptr};\n\
extern \"C\"\n\
__attribute__((export_name(\"loom_init\")))\n\
void loom_init() { loom_main(s_argc, s_argv); }\n"
    )
else()
    set(_LOOM_NATIVE_MAIN_SRC "${CMAKE_BINARY_DIR}/_loom_native_main.cpp")
    file(CONFIGURE OUTPUT "${_LOOM_NATIVE_MAIN_SRC}" CONTENT
"// Auto-generated by LoomHelpers.cmake — do not edit.\n\
extern int loom_main(int argc, char *argv[]);\n\
int main(int argc, char *argv[]) { return loom_main(argc, argv); }\n"
    )
endif()
