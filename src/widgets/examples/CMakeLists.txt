add_executable(widget widget.cpp)
target_link_libraries(widget PRIVATE loom-widgets)

add_executable(nested_windows nested_windows.cpp)
target_link_libraries(nested_windows PRIVATE loom-widgets)

add_executable(breadcrumb_browser
    breadcrumb_browser.cpp
)
target_link_libraries(breadcrumb_browser PRIVATE loom-widgets loom-models)

add_executable(merge_materials
    merge_materials.cpp
)
target_include_directories(merge_materials PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/models/src
)
target_link_libraries(merge_materials PRIVATE loom-widgets loom-models loom-models-private)
