# cmake_minimum_required(VERSION 3.20)
# project(models LANGUAGES CXX)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_library(models
#     src/tzeventemitter.cpp
#     src/tzabstractitemmodel.cpp
#     src/tztreenode.cpp
#     src/tztreemodel.cpp
#     src/tzbreadcrumbmodel.cpp
# )

# target_include_directories(models
#     PUBLIC
#         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/src
# )

# option(BUILD_TESTS "Build tests" ON)

# if(BUILD_TESTS)
#     add_subdirectory(external/googletest EXCLUDE_FROM_ALL)
#     add_subdirectory(tests)
# endif()

add_library(loom-models
    src/tzabstractitemmodel.cpp
    src/tzabstractlistmodel.cpp
)
target_include_directories(loom-models
    PUBLIC  include
    PRIVATE src
)
target_link_libraries(loom-models
    PUBLIC  loom-core
    PRIVATE loom-core-private
)

add_library(loom-models-private INTERFACE)
target_include_directories(loom-models-private INTERFACE src)
target_sources(loom-models-private INTERFACE
    src/tzabstractitemmodel_p.hpp
)

if(LOOM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LOOM_BUILD_TESTS)
    add_subdirectory(tests)
endif()
