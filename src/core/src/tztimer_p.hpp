#ifndef TZTIMER_P
#define TZTIMER_P

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tztimer.hpp>

class TzAbstractEventDispatcher;

class TzTimerPrivate
{
    TZ_DECLARE_PUBLIC(TzTimer)
public:
    explicit TzTimerPrivate(TzAbstractEventDispatcher *eventDispatcher);

private:
    TzTimer *q_ptr{ nullptr };

    TzAbstractEventDispatcher *eventDispatcher{ nullptr };
    TzTimer::TimerHandle handle{ nullptr };
    TzTimer::TimerInterval interval{ 0 };
    bool singleShot{ false };
    bool active{ false };
    TzTimer::TimerCallback callback;
};

#endif // TZTIMER_P
