loom_add_executable(console console.cpp)
target_link_libraries(console PRIVATE loom-core)

loom_add_executable(stack_lifetime stack_lifetime.cpp)
target_link_libraries(stack_lifetime PRIVATE loom-core)
