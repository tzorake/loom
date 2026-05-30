#pragma once

#include <loom/tzabstractplatformintegration.hpp>

class TzWebPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    TzAbstractEventDispatcher *createEventDispatcher() override;
    TzAbstractConsoleInput    *createConsoleInput()    override;
    std::string                name()            const override;
};
