#ifndef TZTIMER_P
#define TZTIMER_P

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tztimer.hpp>

class TzAbstractEventDispatcher;

class TzTimerPrivate
{
    TZ_DECLARE_PUBLIC(TzTimer)
public:
    TzTimerPrivate();

private:
    TzTimer *q_ptr{nullptr};

    TzTimer::TimerHandle handle{nullptr};
    TzTimer::TimerInterval interval{0};
    bool singleShot{false};
    bool active{false};
    TzTimer::TimerCallback callback;
};

#endif // TZTIMER_P
