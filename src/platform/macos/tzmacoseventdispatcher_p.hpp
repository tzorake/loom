#ifndef TZMACOSEVENTDISPATCHER_P_HPP
#define TZMACOSEVENTDISPATCHER_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzmacoseventdispatcher.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <memory>
#include <unordered_map>

class TzMacosEventDispatcherPrivate
{
    TZ_DECLARE_PUBLIC(TzMacosEventDispatcher)
public:
    TzMacosEventDispatcherPrivate();

    TzMacosEventDispatcher *q_ptr{ nullptr };

    CFRunLoopRef runLoop{ nullptr };
    bool interrupted{ false };

    CFRunLoopObserverRef preWaitObserver{ nullptr };
    TzAbstractEventDispatcher::PreWaitCallback preWaitCallback;

    struct TimerWrapper {
        CFRunLoopTimerRef timer{ nullptr };
        TzAbstractEventDispatcher::TimerCallback callback;
        bool singleShot{ false };
        TzMacosEventDispatcher *eventDispatcher{ nullptr };
    };
    using TimerWrapperPtr = std::unique_ptr<TimerWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::TimerHandle, TimerWrapperPtr> timerMap;

    struct NotifyWrapper {
        CFSocketRef socket{ nullptr };
        CFRunLoopSourceRef source{ nullptr };
        TzAbstractEventDispatcher::NotifyCallback callback;
        TzMacosEventDispatcher *eventDispatcher{ nullptr };
    };
    using NotifyWrapperPtr = std::unique_ptr<NotifyWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::NotifyHandle, NotifyWrapperPtr> notifyMap;
};

#endif // TZMACOSEVENTDISPATCHER_P_HPP
