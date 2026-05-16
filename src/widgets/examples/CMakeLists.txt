add_executable(widget widget.cpp)
target_link_libraries(widget PRIVATE loom-widgets)

add_executable(nested_windows nested_windows.cpp)
target_link_libraries(nested_windows PRIVATE loom-widgets)
