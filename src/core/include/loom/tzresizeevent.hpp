#ifndef TZRESIZEEVENT_HPP
#define TZRESIZEEVENT_HPP

#include <event-loop/tzevent.hpp>

class TzResizeEvent : public TzEvent
{
public:
    TzResizeEvent(int width, int height)
        : TzEvent(TzEvent::WindowResize), m_width(width), m_height(height) {}

    int width()  const { return m_width;  }
    int height() const { return m_height; }

    TzResizeEvent *clone() const override { return new TzResizeEvent(*this); }

private:
    int m_width;
    int m_height;
};

#endif // TZRESIZEEVENT_HPP
