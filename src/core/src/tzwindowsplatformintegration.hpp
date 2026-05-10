#ifndef TZWINDOWSPLATFORMINTEGRATION_HPP
#define TZWINDOWSPLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzWindowsPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput    *createConsoleInput()    override;
    virtual TzAbstractWindow          *createWindow(int width, int height) override;
    virtual std::string                name() const            override;
};

#endif // TZWINDOWSPLATFORMINTEGRATION_HPP
