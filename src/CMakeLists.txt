add_subdirectory(core)
add_subdirectory(widgets)
add_subdirectory(models)
add_subdirectory(rhi)

add_library(loom INTERFACE)
target_link_libraries(loom INTERFACE loom-core loom-widgets loom-models loom-rhi)
