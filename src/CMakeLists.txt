add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(models)
add_subdirectory(rhi)

add_library(loom INTERFACE)
target_link_libraries(loom INTERFACE loom-core loom-widgets loom-models loom-rhi)

if(LOOM_BUILD_EXAMPLES AND CMAKE_SYSTEM_NAME STREQUAL "WASI")
    set(_gallery_examples widget window nested_windows breadcrumb_browser loom_quad)
    loom_add_gallery(loom_gallery EXAMPLES ${_gallery_examples})
endif()
