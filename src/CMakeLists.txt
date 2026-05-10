add_subdirectory(core)
add_subdirectory(widgets)

add_library(loom INTERFACE)
target_link_libraries(loom INTERFACE loom-core loom-widgets)
