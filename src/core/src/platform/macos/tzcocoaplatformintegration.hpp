#ifndef TZCOCOAPLATFORMINTEGRATION_HPP
#define TZCOCOAPLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzCocoaPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput *createConsoleInput() override;
    virtual std::string name() const override;
};

#endif // TZCOCOAPLATFORMINTEGRATION_HPP
