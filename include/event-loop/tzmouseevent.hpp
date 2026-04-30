#ifndef TZMOUSEEVENT_HPP
#define TZMOUSEEVENT_HPP

enum class MouseButton
{
    None   = 0,
    Left   = 1 << 0,
    Right  = 1 << 1,
    Middle = 1 << 2,
};

enum class MouseEventType
{
    Move,
    ButtonPress,
    ButtonRelease,
    Scroll,
};

struct TzMouseEvent
{
    MouseEventType type{ MouseEventType::Move };
    MouseButton button{ MouseButton::None };
    double x{ 0.0 };
    double y{ 0.0 };
    double scrollDx{ 0.0 };
    double scrollDy{ 0.0 };
    int modifiers{ 0 };
};

#endif // TZMOUSEEVENT_HPP
