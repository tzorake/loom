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

    struct TimerWrapper {
        CFRunLoopTimerRef timer{ nullptr };
        TzAbstractEventDispatcher::TimerCallback callback;
        bool singleShot{ false };
        TzMacosEventDispatcher *dispatcher{ nullptr };
    };
    using TimerWrapperPtr = std::unique_ptr<TimerWrapper>;

    std::unordered_map<TzAbstractEventDispatcher::TimerHandle, TimerWrapperPtr> timerMap;
};

#endif // TZMACOSEVENTDISPATCHER_P_HPP
