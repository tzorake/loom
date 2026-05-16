cmake_minimum_required(VERSION 3.20)
project(models LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(models
    src/tzeventemitter.cpp
    src/tzabstractitemmodel.cpp
    src/tztreenode.cpp
    src/tztreemodel.cpp
    src/tzbreadcrumbmodel.cpp
)

target_include_directories(models
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    add_subdirectory(external/googletest EXCLUDE_FROM_ALL)
    add_subdirectory(tests)
endif()
