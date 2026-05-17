#include <loom/tzmouseevent.hpp>

TzMouseEvent::TzMouseEvent(int type, MouseButton button, double x, double y, KeyModifiers modifiers,
                           double scrollDx, double scrollDy)
    : TzEvent(type)
    , m_button(button)
    , m_x(x)
    , m_y(y)
    , m_scrollDx(scrollDx)
    , m_scrollDy(scrollDy)
    , m_modifiers(modifiers)
{}

MouseButton TzMouseEvent::button() const
{
    return m_button;
}

double TzMouseEvent::x() const
{
    return m_x;
}

double TzMouseEvent::y() const
{
    return m_y;
}

double TzMouseEvent::scrollDx() const
{
    return m_scrollDx;
}

double TzMouseEvent::scrollDy() const
{
    return m_scrollDy;
}

KeyModifiers TzMouseEvent::modifiers() const
{
    return m_modifiers;
}

TzMouseEvent *TzMouseEvent::clone() const
{
    return new TzMouseEvent(*this);
}
