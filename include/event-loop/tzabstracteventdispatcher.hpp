#ifndef TZABSTRACTEVENTDISPATCHER_HPP
#define TZABSTRACTEVENTDISPATCHER_HPP

#include <chrono>
#include <functional>

class TzAbstractEventDispatcher
{
public:
    using TimerHandle = void *;
    using TimerInterval = std::chrono::milliseconds;
    using TimerCallback = std::function<void()>;

    virtual ~TzAbstractEventDispatcher();

    virtual void processEvents() = 0;
    virtual void interrupt() = 0;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot, TimerCallback callback) = 0;
    virtual void unregisterTimer(TimerHandle) = 0;
};

#endif // TZABSTRACTEVENTDISPATCHER_HPP
