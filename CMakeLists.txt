cmake_minimum_required(VERSION 3.25)
project(loom CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LOOM_BUILD_EXAMPLES "Build example applications" ON)

add_subdirectory(src)

if(LOOM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
