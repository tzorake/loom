add_library(event-loop-widgets
    src/tzwidget.cpp
    src/tzanchors.cpp
    src/tzscene.cpp
    src/tzwindow.cpp
)
target_include_directories(event-loop-widgets
    PUBLIC  include
    PRIVATE src
)
target_link_libraries(event-loop-widgets
    PUBLIC  event-loop
    PRIVATE event-loop-private
)

add_library(event-loop-widgets-private INTERFACE)
target_include_directories(event-loop-widgets-private INTERFACE src)
target_sources(event-loop-widgets-private INTERFACE
    src/tzwidget_p.hpp
    src/tzanchors_p.hpp
    src/tzscene_p.hpp
    src/tzwindow_p.hpp
)

if(APPLE)
    target_link_libraries(event-loop-widgets PUBLIC "-framework Cocoa")
endif()
