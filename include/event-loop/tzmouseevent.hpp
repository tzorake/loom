#ifndef TZMOUSEEVENT_HPP
#define TZMOUSEEVENT_HPP

#include <event-loop/tzevent.hpp>
#include <event-loop/tzkeyevent.hpp>

enum class MouseButton
{
    None   = 0,
    Left   = 1 << 0,
    Right  = 1 << 1,
    Middle = 1 << 2,
};

class TzMouseEvent : public TzEvent
{
public:
    TzMouseEvent(int type, MouseButton button, double x, double y,
                 KeyModifiers modifiers, double scrollDx = 0.0, double scrollDy = 0.0);

    MouseButton  button()    const;
    double       x()         const;
    double       y()         const;
    double       scrollDx()  const;
    double       scrollDy()  const;
    KeyModifiers modifiers() const;

    TzMouseEvent *clone() const override;

private:
    MouseButton  m_button;
    double       m_x;
    double       m_y;
    double       m_scrollDx;
    double       m_scrollDy;
    KeyModifiers m_modifiers;
};

#endif // TZMOUSEEVENT_HPP
