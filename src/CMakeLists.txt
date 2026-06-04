# Declare the gallery BEFORE subdirectories so LOOM_GALLERY_DIR is set in the
# cache when loom_add_executable() runs inside each example subdirectory.
if(LOOM_BUILD_EXAMPLES AND CMAKE_SYSTEM_NAME STREQUAL "WASI")
    set(_gallery_examples stack_lifetime widget window nested_windows breadcrumb_browser loom_quad)
    loom_add_gallery(loom_gallery EXAMPLES ${_gallery_examples})
endif()

add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(models)
add_subdirectory(rhi)

add_library(loom INTERFACE)
target_link_libraries(loom INTERFACE loom-core loom-widgets loom-models loom-rhi)

# Finish the gallery AFTER subdirectories so example targets are defined.
loom_gallery_finish()
