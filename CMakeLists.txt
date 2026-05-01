cmake_minimum_required(VERSION 3.25)
project(event-loop CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EVENT_LOOP_BUILD_EXAMPLES "Build example applications" ON)

add_library(event-loop
    src/tzabstractplatformintegration.cpp
    src/tzabstracteventdispatcher.cpp
    src/tzabstractconsoleinput.cpp
    src/tzabstractwindow.cpp
    src/tzeventloop.cpp
    src/tztimer.cpp
    src/tzsocketnotifier.cpp
    src/tzkeyboardhandler.cpp
    src/tzsignalhandler.cpp
    src/tzevent.cpp
    src/tzkeyevent.cpp
    src/tzmouseevent.cpp
    src/tzcoreapplication.cpp
    src/tzguiapplication.cpp
    src/tzobject.cpp
)
target_include_directories(event-loop PUBLIC include)

if(APPLE)
    target_sources(event-loop PRIVATE
        src/platform/macos/tzmacoseventdispatcher.cpp
        src/platform/macos/tzmacosconsoleinput.cpp
        src/platform/macos/tzmacoswindow.cpp
    )
    target_link_libraries(event-loop PUBLIC
        "-framework CoreFoundation"
        "-framework CoreGraphics"
    )
endif()

if(EVENT_LOOP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
