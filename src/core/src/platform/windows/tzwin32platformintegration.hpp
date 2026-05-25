#ifndef TZWIN32PLATFORMINTEGRATION_HPP
#define TZWIN32PLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzWin32PlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput *createConsoleInput() override;
    virtual std::string name() const override;
};

#endif // TZWIN32PLATFORMINTEGRATION_HPP
