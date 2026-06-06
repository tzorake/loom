cmake_minimum_required(VERSION 3.25)
project(loom C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LOOM_BUILD_EXAMPLES "Build example applications" ON)
option(LOOM_BUILD_TESTS "Build tests" ON)

include(cmake/LoomHelpers.cmake)

if(LOOM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(external/googletest EXCLUDE_FROM_ALL)
endif()

add_subdirectory(src)
