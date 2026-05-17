#ifndef TZCOCOAEVENTDISPATCHER_P_HPP
#define TZCOCOAEVENTDISPATCHER_P_HPP

#include "tzcocoaeventdispatcher.hpp"
#include <loom/tzclasshelpermacros.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <memory>
#include <unordered_map>

class TzCocoaEventDispatcherPrivate
{
    TZ_DECLARE_PUBLIC(TzCocoaEventDispatcher)
public:
    TzCocoaEventDispatcherPrivate();

    TzCocoaEventDispatcher *q_ptr{nullptr};

    CFRunLoopRef runLoop{nullptr};
    bool interrupted{false};

    CFRunLoopObserverRef preWaitObserver{nullptr};
    TzAbstractEventDispatcher::PreWaitCallback preWaitCallback;

    struct TimerWrapper
    {
        CFRunLoopTimerRef timer{nullptr};
        TzAbstractEventDispatcher::TimerCallback callback;
        bool singleShot{false};
        TzCocoaEventDispatcher *eventDispatcher{nullptr};
    };
    using TimerWrapperPtr = std::unique_ptr<TimerWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::TimerHandle, TimerWrapperPtr> timerMap;

    struct NotifyWrapper
    {
        CFSocketRef socket{nullptr};
        CFRunLoopSourceRef source{nullptr};
        TzAbstractEventDispatcher::NotifyCallback callback;
        TzCocoaEventDispatcher *eventDispatcher{nullptr};
    };
    using NotifyWrapperPtr = std::unique_ptr<NotifyWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::NotifyHandle, NotifyWrapperPtr> notifyMap;
};

#endif // TZCOCOAEVENTDISPATCHER_P_HPP
