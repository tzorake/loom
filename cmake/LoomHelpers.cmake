cmake_minimum_required(VERSION 3.25)

set(_LOOM_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

unset(LOOM_WEB_DIR CACHE)
if(NOT DEFINED LOOM_WEB_DIR)
    set(LOOM_WEB_DIR "${CMAKE_CURRENT_LIST_DIR}/../src/core/src/platform/web")
endif()

function(loom_add_executable target)
    set(_sources ${ARGN})

    if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
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
        add_executable(${target} ${_sources} "${_LOOM_NATIVE_MAIN_SRC}")
        target_link_libraries(${target} PRIVATE loom-widgets)
    endif()
endfunction()

function(loom_add_gallery gallery_target)
    cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "EXAMPLES")

    if(NOT CMAKE_SYSTEM_NAME STREQUAL "WASI")
        return()
    endif()

    if(NOT ARG_EXAMPLES)
        message(FATAL_ERROR "loom_add_gallery: EXAMPLES list is empty")
    endif()

    set(_gallery_dir "${CMAKE_BINARY_DIR}/gallery")

    # Build the JS array of { id, label, path } descriptors.
    set(_js_items "")
    foreach(_ex IN LISTS ARG_EXAMPLES)
        string(REGEX REPLACE "^loom_" "" _label "${_ex}")
        string(REPLACE "_" " " _label "${_label}")
        if(_js_items)
            string(APPEND _js_items ",\n      ")
        endif()
        string(APPEND _js_items
            "{ id: '${_ex}', label: '${_label}', path: 'examples/${_ex}' }")
    endforeach()

    # Generate gallery/index.html from the template at configure time.
    # The only variable substituted is @GALLERY_EXAMPLES@.
    set(GALLERY_EXAMPLES "${_js_items}")
    configure_file(
        "${LOOM_WEB_DIR}/gallery.html"
        "${_gallery_dir}/index.html"
        @ONLY
    )

    # Custom target: depends on all listed examples.
    add_custom_target(${gallery_target} ALL)
    add_dependencies(${gallery_target} ${ARG_EXAMPLES})

    # After each example is built, mirror its output directory into
    # gallery/examples/<name>/.  copy_directory creates the destination if
    # missing, and is a no-op when all files are already up to date.
    foreach(_ex IN LISTS ARG_EXAMPLES)
        add_custom_command(TARGET ${gallery_target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "$<TARGET_FILE_DIR:${_ex}>"
                "${_gallery_dir}/examples/${_ex}"
            COMMENT "Gallery: syncing ${_ex}"
        )
    endforeach()

    message(STATUS "Loom gallery '${gallery_target}' → ${_gallery_dir}")
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
