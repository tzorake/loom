# Each call to loom_add_executable() works for both native and web (WASI)
# builds.  The helper (cmake/LoomHelpers.cmake) auto-generates the correct
# entry-point wrapper (main() on native, loom_init() on WASI) and, for WASI,
# copies index.html + loom.js alongside the .wasm output.
#
# Source files must define:
#   int loom_main(int argc, char *argv[]);
#
# Web builds: serve the output directory with:
#   python3 -m http.server 8080 --directory <build>/src/widgets/examples

loom_add_executable(window window.cpp)

loom_add_executable(widget widget.cpp)

loom_add_executable(nested_windows nested_windows.cpp)

loom_add_executable(breadcrumb_browser breadcrumb_browser.cpp)
target_link_libraries(breadcrumb_browser PRIVATE loom-models)

loom_add_executable(bouncing_balls bouncing_balls.cpp)

loom_add_executable(merge_materials merge_materials.cpp)
target_include_directories(merge_materials PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/models/src
)
target_link_libraries(merge_materials PRIVATE loom-models loom-models-private)
