#ifndef TZPAINTEVENT_HPP
#define TZPAINTEVENT_HPP

#include <event-loop/tzevent.hpp>

class TzPainter;

class TzPaintEvent : public TzEvent
{
public:
    explicit TzPaintEvent(TzPainter *painter)
        : TzEvent(TzEvent::Paint)
        , m_painter(painter)
    {}

    TzPainter *painter() const { return m_painter; }

    // Paint events are not cloned — they are always delivered synchronously
    // and the painter pointer is only valid during the call.
    TzPaintEvent *clone() const override { return new TzPaintEvent(*this); }

private:
    TzPainter *m_painter;
};

#endif // TZPAINTEVENT_HPP
