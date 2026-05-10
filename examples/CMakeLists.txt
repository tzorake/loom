add_executable(console console.cpp)
target_link_libraries(console PRIVATE loom)

add_executable(window window.cpp)
target_link_libraries(window PRIVATE loom "-framework Cocoa")

add_executable(widget widget.cpp)
target_link_libraries(widget PRIVATE loom-widgets "-framework Cocoa")

add_executable(nested_windows nested_windows.cpp)
target_link_libraries(nested_windows PRIVATE loom-widgets "-framework Cocoa")
