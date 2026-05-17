#ifndef TZFOCUSEVENT_HPP
#define TZFOCUSEVENT_HPP

#include <loom/tzevent.hpp>

class TzFocusEvent : public TzEvent
{
public:
    explicit TzFocusEvent(int type)
        : TzEvent(type)
    {}
    TzFocusEvent *clone() const override { return new TzFocusEvent(*this); }
};

#endif // TZFOCUSEVENT_HPP
