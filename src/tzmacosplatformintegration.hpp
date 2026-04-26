#ifndef TZMACOSPLATFORMINTEGRATION_HPP
#define TZMACOSPLATFORMINTEGRATION_HPP

#include <event-loop/tzabstractplatformintegration.hpp>

class TzMacosPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput *createConsoleInput() override;
    virtual std::string name() const override;
};

#endif // TZMACOSPLATFORMINTEGRATION_HPP
