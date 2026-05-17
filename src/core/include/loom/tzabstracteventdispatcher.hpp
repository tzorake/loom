#ifndef TZABSTRACTEVENTDISPATCHER_HPP
#define TZABSTRACTEVENTDISPATCHER_HPP

#include <chrono>
#include <functional>
#include <utility>

class TzAbstractEventDispatcher
{
public:
    using TimerHandle = void *;
    using TimerInterval = std::chrono::milliseconds;
    using TimerCallback = std::function<void()>;

    using NotifyHandle = void *;
    using NotifyCallback = std::function<void(int)>;

    virtual ~TzAbstractEventDispatcher();

    using PreWaitCallback = std::function<void()>;

    virtual void processEvents() = 0;
    virtual void interrupt() = 0;
    virtual void wakeUp() = 0;
    virtual void setPreWaitCallback(PreWaitCallback callback) = 0;

    virtual TimerHandle registerTimer(TimerInterval interval, bool singleShot,
                                      TimerCallback callback)
        = 0;
    virtual void unregisterTimer(TimerHandle handle) = 0;

    virtual NotifyHandle registerSocketNotifier(int fd, NotifyCallback callback) = 0;
    virtual void unregisterSocketNotifier(NotifyHandle handle) = 0;
};

#endif // TZABSTRACTEVENTDISPATCHER_HPP
