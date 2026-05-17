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
    src/tzeventemitter.cpp
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
    src/tzsignalpipe.hpp
    src/tzeventemitter_p.hpp
)

if(APPLE)
    target_sources(loom-core PRIVATE
        src/platform/macos/tzcocoaeventdispatcher.cpp
        src/platform/macos/tzcocoaconsoleinput.cpp
        src/platform/macos/tzcocoawindow.cpp
        src/platform/macos/tzcocoasignalpipe.cpp
    )
    target_include_directories(loom-core PRIVATE src/platform/macos)
    target_sources(loom-core-private INTERFACE
        src/platform/macos/tzcocoaeventdispatcher.hpp
        src/platform/macos/tzcocoaeventdispatcher_p.hpp
        src/platform/macos/tzcocoaconsoleinput.hpp
        src/platform/macos/tzcocoaconsoleinput_p.hpp
        src/platform/macos/tzcocoawindow.hpp
        src/platform/macos/tzcocoawindow_p.hpp
        src/platform/macos/tzobjcutils.hpp
        src/tzcocoaplatformintegration.hpp
    )
    target_link_libraries(loom-core PUBLIC
        "-framework Cocoa"
        "-framework CoreFoundation"
        "-framework CoreGraphics"
    )
elseif(WIN32)
    target_sources(loom-core PRIVATE
        src/platform/windows/tzwin32eventdispatcher.cpp
        src/platform/windows/tzwin32consoleinput.cpp
        src/platform/windows/tzwin32window.cpp
        src/platform/windows/tzwin32signalpipe.cpp
    )
    target_include_directories(loom-core PRIVATE src/platform/windows)
    target_sources(loom-core-private INTERFACE
        src/platform/windows/tzwin32eventdispatcher.hpp
        src/platform/windows/tzwin32eventdispatcher_p.hpp
        src/platform/windows/tzwin32consoleinput.hpp
        src/platform/windows/tzwin32consoleinput_p.hpp
        src/platform/windows/tzwin32window.hpp
        src/platform/windows/tzwin32window_p.hpp
        src/tzwin32platformintegration.hpp
    )
    target_link_libraries(loom-core PUBLIC
        user32 gdi32
    )
elseif(UNIX AND NOT APPLE)
    set(LOOM_LINUX_BACKEND "auto" CACHE STRING
        "Linux display backend: auto (detect from XDG_SESSION_TYPE), wayland, x11")
    set_property(CACHE LOOM_LINUX_BACKEND PROPERTY STRINGS auto wayland x11)

    if(LOOM_LINUX_BACKEND STREQUAL "auto")
        set(_loom_session "$ENV{XDG_SESSION_TYPE}")
        if(_loom_session STREQUAL "x11")
            set(_loom_backend "x11")
        else()
            set(_loom_backend "wayland")
        endif()
        message(STATUS "Loom: auto-detected Linux backend '${_loom_backend}'"
                       " (XDG_SESSION_TYPE='${_loom_session}')")
    else()
        set(_loom_backend "${LOOM_LINUX_BACKEND}")
        message(STATUS "Loom: using Linux backend '${_loom_backend}' (manually specified)")
    endif()

    if(_loom_backend STREQUAL "x11")
        target_sources(loom-core PRIVATE
            src/platform/linux/tzx11eventdispatcher.cpp
            src/platform/linux/tzx11consoleinput.cpp
            src/platform/linux/tzx11window.cpp
            src/platform/linux/tzx11globals.cpp
            src/platform/linux/tzx11signalpipe.cpp
        )
        target_include_directories(loom-core PRIVATE src/platform/linux)
        target_sources(loom-core-private INTERFACE
            src/platform/linux/tzx11eventdispatcher.hpp
            src/platform/linux/tzx11eventdispatcher_p.hpp
            src/platform/linux/tzx11consoleinput.hpp
            src/platform/linux/tzx11consoleinput_p.hpp
            src/platform/linux/tzx11window.hpp
            src/platform/linux/tzx11window_p.hpp
            src/platform/linux/tzx11globals.hpp
            src/tzx11platformintegration.hpp
        )
        target_compile_definitions(loom-core PUBLIC LOOM_BACKEND_X11)
        target_link_libraries(loom-core PUBLIC X11)
    else() # wayland
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
endif()

if(LOOM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LOOM_BUILD_TESTS)
    add_subdirectory(tests)
endif()
