add_executable(list_model list_model.cpp)
target_link_libraries(list_model PRIVATE loom-models)

add_executable(tree_model
    tree_model.cpp
)
target_include_directories(tree_model PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tree_model PRIVATE loom-models loom-models-private)
