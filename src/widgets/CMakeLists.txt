add_library(loom-widgets
    src/tzwidget.cpp
    src/tzanchors.cpp
    src/tzscene.cpp
    src/tzwindow.cpp
)
target_include_directories(loom-widgets
    PUBLIC  include
    PRIVATE src
)
target_link_libraries(loom-widgets
    PUBLIC  loom
    PRIVATE loom-private
)

add_library(loom-widgets-private INTERFACE)
target_include_directories(loom-widgets-private INTERFACE src)
target_sources(loom-widgets-private INTERFACE
    src/tzwidget_p.hpp
    src/tzanchors_p.hpp
    src/tzscene_p.hpp
    src/tzwindow_p.hpp
)

if(APPLE)
    target_link_libraries(loom-widgets PUBLIC "-framework Cocoa")
endif()
