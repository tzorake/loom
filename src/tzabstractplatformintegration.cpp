#include <event-loop/tzabstractplatformintegration.hpp>

#ifdef __APPLE__
#include <event-loop/tzmacoseventdispatcher.hpp>
#include <event-loop/tzmacosconsoleinput.hpp>
#include "tzmacosplatformintegration.hpp"

TzAbstractEventDispatcher *TzMacosPlatformIntegration::createEventDispatcher()
{
    return new TzMacosEventDispatcher;
}

TzAbstractConsoleInput *TzMacosPlatformIntegration::createConsoleInput()
{
    return new TzMacosConsoleInput;
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
