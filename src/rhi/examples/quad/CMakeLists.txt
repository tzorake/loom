cmake_minimum_required(VERSION 3.21)

if(CMAKE_SYSTEM_NAME STREQUAL "WASI" OR LOOM_PLATFORM_WEB)
    # Web target: produce a WASM executable that loom.js boots.
    add_executable(loom_quad quad.cpp)
    target_link_libraries(loom_quad PRIVATE loom-rhi loom)
    set_target_properties(loom_quad PROPERTIES
        OUTPUT_NAME loom_app
        SUFFIX      ".wasm"
    )
else()
    add_executable(loom_quad quad.cpp)
    target_link_libraries(loom_quad PRIVATE loom-rhi loom)
endif()
