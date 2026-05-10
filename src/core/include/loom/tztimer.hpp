#ifndef TZTIMER_HPP
#define TZTIMER_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <chrono>
#include <functional>
#include <memory>

class TzAbstractEventDispatcher;
class TzTimerPrivate;

class TzTimer
{
    TZ_DECLARE_PRIVATE(TzTimer)
public:
    using TimerHandle = void *;
    using TimerInterval = std::chrono::milliseconds;
    using TimerCallback = std::function<void()>;

    explicit TzTimer(TzAbstractEventDispatcher *eventDispatcher);
    ~TzTimer();

    TimerHandle handle() const;

    void setInterval(TimerInterval interval);
    TimerInterval interval() const;

    void setSingleShot(bool singleShot);
    bool isSingleShot() const;

    void setCallback(TimerCallback callback);

    bool isActive() const;

    /**
     * @brief Starts (or restarts) the timer with the configured interval.
     * @throws std::runtime_error if no event dispatcher has been set.
     * @throws std::runtime_error if no callback has been set.
     */
    void start();
    void stop();

    static TzTimer *singleShot(TzAbstractEventDispatcher *eventDispatcher, TimerInterval interval, TimerCallback callback);
    static TzTimer *repeat(TzAbstractEventDispatcher *eventDispatcher, TimerInterval interval, TimerCallback callback);

private:
    std::unique_ptr<TzTimerPrivate> d_ptr;
};

#endif // TZTIMER_HPP
