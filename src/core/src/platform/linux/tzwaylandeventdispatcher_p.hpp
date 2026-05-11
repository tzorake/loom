#ifndef TZWAYLANDEVENTDISPATCHER_P_HPP
#define TZWAYLANDEVENTDISPATCHER_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include "tzwaylandeventdispatcher.hpp"

#include <wayland-client.h>
#include <atomic>
#include <memory>
#include <unordered_map>

class TzWaylandEventDispatcherPrivate
{
    TZ_DECLARE_PUBLIC(TzWaylandEventDispatcher)
public:
    TzWaylandEventDispatcherPrivate();
    ~TzWaylandEventDispatcherPrivate();

    TzWaylandEventDispatcher *q_ptr { nullptr };

    int epollFd    { -1 };
    int wakeFd     { -1 };
    int waylandFd  { -1 };           // -1 until setWaylandDisplay() is called
    wl_display *waylandDisplay { nullptr };

    std::atomic<bool> interrupted { false };

    TzAbstractEventDispatcher::PreWaitCallback preWaitCallback;

    struct TimerWrapper {
        int  timerFd  { -1 };
        bool singleShot { false };
        TzAbstractEventDispatcher::TimerCallback callback;
    };
    using TimerWrapperPtr = std::unique_ptr<TimerWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::TimerHandle, TimerWrapperPtr> timerMap;

    struct NotifyWrapper {
        int  fd { -1 };
        TzAbstractEventDispatcher::NotifyCallback callback;
    };
    using NotifyWrapperPtr = std::unique_ptr<NotifyWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::NotifyHandle, NotifyWrapperPtr> notifyMap;
};

#endif // TZWAYLANDEVENTDISPATCHER_P_HPP
