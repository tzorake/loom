#include <loom/tzabstractplatformintegration.hpp>

#ifdef __APPLE__
#include "tzmacoseventdispatcher.hpp"
#include "tzmacosconsoleinput.hpp"
#include "tzmacoswindow.hpp"
#include "tzmacosplatformintegration.hpp"

TzAbstractEventDispatcher *TzMacosPlatformIntegration::createEventDispatcher()
{
    return new TzMacosEventDispatcher;
}

TzAbstractConsoleInput *TzMacosPlatformIntegration::createConsoleInput()
{
    return new TzMacosConsoleInput;
}

TzAbstractWindow *TzMacosPlatformIntegration::createWindow(int width, int height)
{
    return new TzMacosWindow(width, height);
}

std::string TzMacosPlatformIntegration::name() const
{
    return "macos";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzMacosPlatformIntegration;
}
#elif _WIN32
#include "platform/windows/tzwindowseventdispatcher.hpp"
#include "platform/windows/tzwindowsconsoleinput.hpp"
#include "platform/windows/tzwindowswindow.hpp"
#include "tzwindowsplatformintegration.hpp"

TzAbstractEventDispatcher *TzWindowsPlatformIntegration::createEventDispatcher()
{
    return new TzWindowsEventDispatcher;
}

TzAbstractConsoleInput *TzWindowsPlatformIntegration::createConsoleInput()
{
    return new TzWindowsConsoleInput;
}

TzAbstractWindow *TzWindowsPlatformIntegration::createWindow(int width, int height)
{
    return new TzWindowsWindow(width, height);
}

std::string TzWindowsPlatformIntegration::name() const
{
    return "windows";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzWindowsPlatformIntegration;
}
#elif __linux__
#ifdef LOOM_BACKEND_X11
#include "platform/linux/tzx11eventdispatcher.hpp"
#include "platform/linux/tzx11consoleinput.hpp"
#include "platform/linux/tzx11window.hpp"
#include "platform/linux/tzx11globals.hpp"
#include "tzx11platformintegration.hpp"

TzAbstractEventDispatcher *TzX11PlatformIntegration::createEventDispatcher()
{
    auto *d = new TzX11EventDispatcher;
    // Wire up the X11 display fd so the event loop can poll it.
    d->setX11Fd(TzX11Globals::instance().fd());
    return d;
}

TzAbstractConsoleInput *TzX11PlatformIntegration::createConsoleInput()
{
    return new TzX11ConsoleInput;
}

TzAbstractWindow *TzX11PlatformIntegration::createWindow(int width, int height)
{
    TzX11Globals::instance(); // ensure display is open (throws if DISPLAY not set)
    return new TzX11Window(width, height);
}

std::string TzX11PlatformIntegration::name() const
{
    return "x11";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzX11PlatformIntegration;
}
#else
#include "platform/linux/tzwaylandeventdispatcher.hpp"
#include "platform/linux/tzwaylandconsoleinput.hpp"
#include "platform/linux/tzwaylandwindow.hpp"
#include "platform/linux/tzwaylandglobals.hpp"
#include "tzwaylandplatformintegration.hpp"

void TzWaylandPlatformIntegration::ensureGlobals()
{
    if (m_globalsReady)
        return;
    m_globalsReady = true;
    TzWaylandGlobals::instance(); // connect to display (throws if unavailable)
    if (m_dispatcher)
        m_dispatcher->setWaylandDisplay(TzWaylandGlobals::instance().display);
}

TzAbstractEventDispatcher *TzWaylandPlatformIntegration::createEventDispatcher()
{
    auto *d = new TzWaylandEventDispatcher;
    m_dispatcher = d;
    return d;
}

TzAbstractConsoleInput *TzWaylandPlatformIntegration::createConsoleInput()
{
    return new TzWaylandConsoleInput;
}

TzAbstractWindow *TzWaylandPlatformIntegration::createWindow(int width, int height)
{
    ensureGlobals();
    return new TzWaylandWindow(width, height);
}

std::string TzWaylandPlatformIntegration::name() const
{
    return "wayland";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzWaylandPlatformIntegration;
}
#endif // LOOM_BACKEND_X11
#else
#endif

TzAbstractPlatformIntegration::~TzAbstractPlatformIntegration()
{
}

TzAbstractPlatformIntegration *createPlatformIntegration()
{
    return createPlatformIntegrationImpl();
}
