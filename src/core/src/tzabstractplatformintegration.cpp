#include <loom/tzabstractplatformintegration.hpp>
#include "tzplatformwindow.hpp"

#ifdef __APPLE__
#include "platform/macos/tzcocoaconsoleinput.hpp"
#include "platform/macos/tzcocoaeventdispatcher.hpp"
#include "platform/macos/tzcocoaplatformintegration.hpp"
#include "platform/macos/tzcocoawindow.hpp"

TzAbstractEventDispatcher *TzCocoaPlatformIntegration::createEventDispatcher()
{
    return new TzCocoaEventDispatcher;
}

TzAbstractConsoleInput *TzCocoaPlatformIntegration::createConsoleInput()
{
    return new TzCocoaConsoleInput;
}

std::string TzCocoaPlatformIntegration::name() const
{
    return "macos";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzCocoaPlatformIntegration;
}

TzPlatformWindow *tzCreatePlatformWindow(int width, int height)
{
    return new TzCocoaWindow(width, height);
}
#elif _WIN32
#include "platform/windows/tzwin32consoleinput.hpp"
#include "platform/windows/tzwin32eventdispatcher.hpp"
#include "platform/windows/tzwin32window.hpp"
#include "platform/windows/tzwin32platformintegration.hpp"

TzAbstractEventDispatcher *TzWin32PlatformIntegration::createEventDispatcher()
{
    return new TzWin32EventDispatcher;
}

TzAbstractConsoleInput *TzWin32PlatformIntegration::createConsoleInput()
{
    return new TzWin32ConsoleInput;
}

std::string TzWin32PlatformIntegration::name() const
{
    return "windows";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzWin32PlatformIntegration;
}

TzPlatformWindow *tzCreatePlatformWindow(int width, int height)
{
    return new TzWin32Window(width, height);
}
#elif __linux__
#ifdef LOOM_BACKEND_X11
#include "platform/linux/tzx11consoleinput.hpp"
#include "platform/linux/tzx11eventdispatcher.hpp"
#include "platform/linux/tzx11globals.hpp"
#include "platform/linux/tzx11window.hpp"
#include "platform/linux/tzx11platformintegration.hpp"

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

std::string TzX11PlatformIntegration::name() const
{
    return "x11";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzX11PlatformIntegration;
}

TzPlatformWindow *tzCreatePlatformWindow(int width, int height)
{
    TzX11Globals::instance(); // ensure display is open (throws if DISPLAY not set)
    return new TzX11Window(width, height);
}
#else
#include "platform/linux/tzwaylandconsoleinput.hpp"
#include "platform/linux/tzwaylandeventdispatcher.hpp"
#include "platform/linux/tzwaylandglobals.hpp"
#include "platform/linux/tzwaylandwindow.hpp"
#include "platform/linux/tzwaylandplatformintegration.hpp"

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

std::string TzWaylandPlatformIntegration::name() const
{
    return "wayland";
}

static TzAbstractPlatformIntegration *createPlatformIntegrationImpl()
{
    return new TzWaylandPlatformIntegration;
}

TzPlatformWindow *tzCreatePlatformWindow(int width, int height)
{
    auto *integration = static_cast<TzWaylandPlatformIntegration *>(
        tzApp->platformIntegration());
    integration->ensureGlobals();
    return new TzWaylandWindow(width, height);
}
#endif // LOOM_BACKEND_X11
#else
TzPlatformWindow *tzCreatePlatformWindow(int /*width*/, int /*height*/)
{
    return nullptr;
}
#endif

TzAbstractPlatformIntegration::~TzAbstractPlatformIntegration() {}

TzAbstractPlatformIntegration *createPlatformIntegration()
{
    return createPlatformIntegrationImpl();
}
