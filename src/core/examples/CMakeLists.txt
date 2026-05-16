add_executable(console console.cpp)
target_link_libraries(console PRIVATE loom-core)

add_executable(window window.cpp)
target_link_libraries(window PRIVATE loom-core)
