add_executable(console console.cpp)
target_link_libraries(console PRIVATE event-loop)

add_executable(window window.cpp)
target_link_libraries(window PRIVATE event-loop "-framework Cocoa")

add_executable(widget widget.cpp)
target_link_libraries(widget PRIVATE event-loop-widgets "-framework Cocoa")

add_executable(nested_windows nested_windows.cpp)
target_link_libraries(nested_windows PRIVATE event-loop-widgets "-framework Cocoa")
