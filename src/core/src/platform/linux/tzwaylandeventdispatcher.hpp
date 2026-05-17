#ifndef TZWAYLANDEVENTDISPATCHER_HPP
#define TZWAYLANDEVENTDISPATCHER_HPP

#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzclasshelpermacros.hpp>

#include <memory>
#include <wayland-client.h>

class TzWaylandEventDispatcherPrivate;

class TzWaylandEventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzWaylandEventDispatcher)
public:
    TzWaylandEventDispatcher();
    virtual ~TzWaylandEventDispatcher() override;

    virtual void processEvents() override;
    virtual void interrupt() override;
    virtual void wakeUp() override;
    virtual void setPreWaitCallback(PreWaitCallback callback) override;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot,
                                      TimerCallback callback) override;
    virtual void unregisterTimer(TimerHandle handle) override;

    virtual NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) override;
    virtual void unregisterSocketNotifier(NotifyHandle handle) override;

    // Called by the platform integration once a Wayland display is available.
    // Console-only apps never call this, so they never touch the display.
    void setWaylandDisplay(wl_display *display);

private:
    std::unique_ptr<TzWaylandEventDispatcherPrivate> d_ptr;
};

#endif // TZWAYLANDEVENTDISPATCHER_HPP
