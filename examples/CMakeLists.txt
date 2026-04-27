add_executable(console console.cpp)
target_link_libraries(console PRIVATE event-loop)

add_executable(window window.cpp)
target_link_libraries(window PRIVATE event-loop "-framework Cocoa")
