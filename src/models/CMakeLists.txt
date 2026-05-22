add_library(loom-models
    src/tzabstractitemmodel.cpp
    src/tzabstractlistmodel.cpp
    src/tzabstractproxymodel.cpp
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
    src/tzabstractproxymodel_p.hpp
)

if(LOOM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LOOM_BUILD_TESTS)
    add_subdirectory(tests)
endif()
