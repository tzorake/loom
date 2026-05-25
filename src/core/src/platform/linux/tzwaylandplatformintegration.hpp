#ifndef TZWAYLANDPLATFORMINTEGRATION_HPP
#define TZWAYLANDPLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzWaylandEventDispatcher;

class TzWaylandPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput *createConsoleInput() override;
    virtual std::string name() const override;

    // Called by tzCreatePlatformWindow before constructing the first window.
    void ensureGlobals();

private:
    // Non-owning reference to the dispatcher created by createEventDispatcher().
    // Used to wire up the Wayland display fd on first ensureGlobals() call.
    TzWaylandEventDispatcher *m_dispatcher{nullptr};
    bool m_globalsReady{false};
};

#endif // TZWAYLANDPLATFORMINTEGRATION_HPP
