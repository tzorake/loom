#ifndef TZWAYLANDPLATFORMINTEGRATION_HPP
#define TZWAYLANDPLATFORMINTEGRATION_HPP

#include <loom/tzabstractplatformintegration.hpp>

class TzWaylandEventDispatcher;

class TzWaylandPlatformIntegration : public TzAbstractPlatformIntegration
{
public:
    virtual TzAbstractEventDispatcher *createEventDispatcher() override;
    virtual TzAbstractConsoleInput    *createConsoleInput()    override;
    virtual TzAbstractWindow          *createWindow(int width, int height) override;
    virtual std::string                name() const            override;

private:
    // Non-owning reference to the dispatcher created by createEventDispatcher().
    // Used to wire up the Wayland display fd on first createWindow() call.
    TzWaylandEventDispatcher *m_dispatcher  { nullptr };
    bool                      m_globalsReady{ false   };

    void ensureGlobals();
};

#endif // TZWAYLANDPLATFORMINTEGRATION_HPP
