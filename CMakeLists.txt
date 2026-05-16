cmake_minimum_required(VERSION 3.25)
project(loom C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LOOM_BUILD_EXAMPLES "Build example applications" ON)

add_subdirectory(src)
