#include <loom/tztimer.hpp>
#include <tztimer_p.hpp>
#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzcoreapplication.hpp>

TzTimerPrivate::TzTimerPrivate()
{
}

TzTimer::TzTimer()
    : d_ptr(new TzTimerPrivate)
{
}

TzTimer::~TzTimer()
{
    stop();
}

TzTimer::TimerHandle TzTimer::handle() const
{
    TZ_D(const TzTimer);
    return d->handle;
}

void TzTimer::setInterval(TimerInterval interval)
{
    TZ_D(TzTimer);
    d->interval = interval;
}

TzTimer::TimerInterval TzTimer::interval() const
{
    TZ_D(const TzTimer);
    return d->interval;
}

void TzTimer::setSingleShot(bool singleShot)
{
    TZ_D(TzTimer);
    d->singleShot = singleShot;
}

bool TzTimer::isSingleShot() const
{
    TZ_D(const TzTimer);
    return d->singleShot;
}

void TzTimer::setCallback(TimerCallback callback)
{
    TZ_D(TzTimer);
    d->callback = std::move(callback);
}

bool TzTimer::isActive() const
{
    TZ_D(const TzTimer);
    return d->active;
}

void TzTimer::start()
{
    TzAbstractEventDispatcher *eventDispatcher = tzApp->eventDispatcher();
    if (!eventDispatcher)
        throw std::runtime_error("TzTimer::start() without event dispatcher");

    stop();
    if (!d_ptr->callback)
        throw std::runtime_error("TzTimer::start() without callback");

    d_ptr->handle = eventDispatcher->registerTimer(d_ptr->interval, d_ptr->singleShot,
        [this]() {
            if (d_ptr->callback)
                d_ptr->callback();

            // If singleShot, the dispatcher’s concrete implementation already
            // auto-unregisters. We mark ourselves inactive.
            if (d_ptr->singleShot) {
                d_ptr->active = false;
                d_ptr->handle = nullptr;
            }
        });
    d_ptr->active = true;
}

void TzTimer::stop()
{
    if (d_ptr->active) {
        TzAbstractEventDispatcher *eventDispatcher = tzApp->eventDispatcher();
        eventDispatcher->unregisterTimer(d_ptr->handle);

        d_ptr->active = false;
        d_ptr->handle = nullptr;
    }
}

TzTimer *TzTimer::singleShot(TimerInterval interval, TimerCallback callback)
{
    TzTimer *t = new TzTimer();
    t->setSingleShot(true);
    t->setInterval(interval);
    t->setCallback(std::move(callback));
    t->start();
    return t;
}

TzTimer *TzTimer::repeat(TimerInterval interval, TimerCallback callback)
{
    TzTimer *t = new TzTimer();
    t->setSingleShot(false);
    t->setInterval(interval);
    t->setCallback(std::move(callback));
    t->start();
    return t;
}
