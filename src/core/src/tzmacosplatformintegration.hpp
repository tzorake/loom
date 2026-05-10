#ifndef TZMACOSPLATFORMINTEGRATION_HPP
#define TZMACOSPLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzMacosPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput *createConsoleInput() override;
    virtual TzAbstractWindow *createWindow(int width, int height) override;
    virtual std::string name() const override;
};

#endif // TZMACOSPLATFORMINTEGRATION_HPP
