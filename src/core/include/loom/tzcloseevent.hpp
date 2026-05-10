#ifndef TZCLOSEEVENT_HPP
#define TZCLOSEEVENT_HPP

#include <loom/tzevent.hpp>

class TzCloseEvent : public TzEvent
{
public:
    TzCloseEvent() : TzEvent(TzEvent::WindowClose) {}

    TzCloseEvent *clone() const override { return new TzCloseEvent(*this); }
};

#endif // TZCLOSEEVENT_HPP
