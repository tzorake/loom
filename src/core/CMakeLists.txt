add_library(loom-core
    src/tzabstractplatformintegration.cpp
    src/tzabstracteventdispatcher.cpp
    src/tzabstractconsoleinput.cpp
    src/tzsurface.cpp
    src/tzplatformsurface.cpp
    src/tzabstractwindow.cpp
    src/tzeventloop.cpp
    src/tztimer.cpp
    src/tzsocketnotifier.cpp
    src/tzkeyboardhandler.cpp
    src/tzsignalhandler.cpp
    src/tzevent.cpp
    src/tzkeyevent.cpp
    src/tzmouseevent.cpp
    src/tzgeometrychangeevent.cpp
    src/tzcoreapplication.cpp
    src/tzguiapplication.cpp
    src/tzobject.cpp
    src/tzpainter.cpp
    src/tzmargins.cpp
    src/tzpoint.cpp
    src/tzrect.cpp
    src/tzsize.cpp
)
target_include_directories(loom-core
    PUBLIC  include
    PRIVATE src
)

add_library(loom-core-private INTERFACE)
target_include_directories(loom-core-private INTERFACE src)
target_sources(loom-core-private INTERFACE
    src/tzabstractwindow_p.hpp
    src/tzcoreapplication_p.hpp
    src/tzeventloop_p.hpp
    src/tzkeyboardhandler_p.hpp
    src/tzobject_p.hpp
    src/tzpainter_p.hpp
    src/tzsignalhandler_p.hpp
    src/tzsocketnotifier_p.hpp
    src/tztimer_p.hpp
    src/tzmacosplatformintegration.hpp
)

if(APPLE)
    target_sources(loom-core PRIVATE
        src/platform/macos/tzmacoseventdispatcher.cpp
        src/platform/macos/tzmacosconsoleinput.cpp
        src/platform/macos/tzmacoswindow.cpp
    )
    target_include_directories(loom-core PRIVATE src/platform/macos)
    target_sources(loom-core-private INTERFACE
        src/platform/macos/tzmacoseventdispatcher.hpp
        src/platform/macos/tzmacoseventdispatcher_p.hpp
        src/platform/macos/tzmacosconsoleinput.hpp
        src/platform/macos/tzmacosconsoleinput_p.hpp
        src/platform/macos/tzmacoswindow.hpp
        src/platform/macos/tzmacoswindow_p.hpp
        src/platform/macos/tzobjcutils.hpp
    )
    target_link_libraries(loom-core PUBLIC
        "-framework CoreFoundation"
        "-framework CoreGraphics"
    )
endif()
