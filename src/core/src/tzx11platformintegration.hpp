#ifndef TZX11PLATFORMINTEGRATION_HPP
#define TZX11PLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzX11PlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    TzAbstractEventDispatcher *createEventDispatcher() override;
    TzAbstractConsoleInput    *createConsoleInput()    override;
    TzAbstractWindow          *createWindow(int width, int height) override;
    std::string                name() const            override;
};

#endif // TZX11PLATFORMINTEGRATION_HPP
