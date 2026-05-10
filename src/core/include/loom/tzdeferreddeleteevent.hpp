#ifndef TZDEFERREDDELETEEVENT_HPP
#define TZDEFERREDDELETEEVENT_HPP

#include <event-loop/tzevent.hpp>

class TzDeferredDeleteEvent : public TzEvent
{
public:
    TzDeferredDeleteEvent() : TzEvent(TzEvent::DeferredDelete) {}

    TzDeferredDeleteEvent *clone() const override { return new TzDeferredDeleteEvent(*this); }
};

#endif // TZDEFERREDDELETEEVENT_HPP
