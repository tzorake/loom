#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tztimer.hpp>

#include "tztimer_p.hpp"

TzTimerPrivate::TzTimerPrivate(TzAbstractEventDispatcher *eventDispatcher)
    : eventDispatcher(eventDispatcher)
{}

TzTimer::TzTimer(TzAbstractEventDispatcher *eventDispatcher)
    : d_ptr(new TzTimerPrivate(eventDispatcher))
{}

TzTimer::~TzTimer()
{
    stop();
}

TzTimer::TimerHandle TzTimer::handle() const
{
    return d_ptr->handle;
}

void TzTimer::setInterval(TimerInterval interval)
{
    d_ptr->interval = interval;
}

TzTimer::TimerInterval TzTimer::interval() const
{
    return d_ptr->interval;
}

void TzTimer::setSingleShot(bool singleShot)
{
    d_ptr->singleShot = singleShot;
}

bool TzTimer::isSingleShot() const
{
    return d_ptr->singleShot;
}

void TzTimer::setCallback(TimerCallback callback)
{
    d_ptr->callback = std::move(callback);
}

bool TzTimer::isActive() const
{
    return d_ptr->active;
}

void TzTimer::start()
{
    if (!d_ptr->eventDispatcher)
        throw std::runtime_error("Timer::start() without event dispatcher");

    stop();
    if (!d_ptr->callback)
        throw std::runtime_error("Timer::start() without callback");

    d_ptr->handle = d_ptr->eventDispatcher->registerTimer(d_ptr->interval, d_ptr->singleShot,
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
        d_ptr->eventDispatcher->unregisterTimer(d_ptr->handle);
        d_ptr->active = false;
        d_ptr->handle = nullptr;
    }
}

TzTimer *TzTimer::singleShot(TzAbstractEventDispatcher *eventDispatcher, TimerInterval interval,
                             TimerCallback callback)
{
    TzTimer *t = new TzTimer(eventDispatcher);
    t->setSingleShot(true);
    t->setInterval(interval);
    t->setCallback(std::move(callback));
    t->start();
    return t;
}

TzTimer *TzTimer::repeat(TzAbstractEventDispatcher *eventDispatcher, TimerInterval interval,
                         TimerCallback callback)
{
    TzTimer *t = new TzTimer(eventDispatcher);
    t->setSingleShot(false);
    t->setInterval(interval);
    t->setCallback(std::move(callback));
    t->start();
    return t;
}
