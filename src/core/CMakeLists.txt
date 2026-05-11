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
        src/platform/macos/tzmacossignalpipe.cpp
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
        "-framework Cocoa"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
    )
elseif(WIN32)
    target_sources(loom-core PRIVATE
        src/platform/windows/tzwindowseventdispatcher.cpp
        src/platform/windows/tzwindowsconsoleinput.cpp
        src/platform/windows/tzwindowswindow.cpp
        src/platform/windows/tzwindowssignalpipe.cpp
    )
    target_include_directories(loom-core PRIVATE src/platform/windows)
    target_sources(loom-core-private INTERFACE
        src/platform/windows/tzwindowseventdispatcher.hpp
        src/platform/windows/tzwindowseventdispatcher_p.hpp
        src/platform/windows/tzwindowsconsoleinput.hpp
        src/platform/windows/tzwindowsconsoleinput_p.hpp
        src/platform/windows/tzwindowswindow.hpp
        src/platform/windows/tzwindowswindow_p.hpp
        src/tzwindowsplatformintegration.hpp
    )
    target_link_libraries(loom-core PUBLIC
        user32 gdi32
    )
elseif(UNIX AND NOT APPLE)
    target_sources(loom-core PRIVATE
        src/platform/linux/tzwaylandeventdispatcher.cpp
        src/platform/linux/tzwaylandconsoleinput.cpp
        src/platform/linux/tzwaylandwindow.cpp
        src/platform/linux/tzwaylandglobals.cpp
        src/platform/linux/tzwaylandsignalpipe.cpp
        src/platform/linux/protocol/xdg-shell-protocol.c
        src/platform/linux/protocol/xdg-decoration-protocol.c
    )
    target_include_directories(loom-core PRIVATE src/platform/linux)
    target_sources(loom-core-private INTERFACE
        src/platform/linux/tzwaylandeventdispatcher.hpp
        src/platform/linux/tzwaylandeventdispatcher_p.hpp
        src/platform/linux/tzwaylandconsoleinput.hpp
        src/platform/linux/tzwaylandconsoleinput_p.hpp
        src/platform/linux/tzwaylandwindow.hpp
        src/platform/linux/tzwaylandwindow_p.hpp
        src/platform/linux/tzwaylandglobals.hpp
        src/tzwaylandplatformintegration.hpp
    )
    target_link_libraries(loom-core PUBLIC
        wayland-client
        wayland-cursor
        xkbcommon
    )
endif()
