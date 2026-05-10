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
#else
#endif

TzAbstractPlatformIntegration::~TzAbstractPlatformIntegration()
{
}

TzAbstractPlatformIntegration *createPlatformIntegration()
{
    return createPlatformIntegrationImpl();
}
