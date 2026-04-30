#ifndef TZKEYEVENT_HPP
#define TZKEYEVENT_HPP

#include <event-loop/tzflags.hpp>

#include <string>

enum class Key
{
    Unknown = 0,
    Escape,
    Enter,
    Tab,
    Backspace,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
};

enum class KeyModifier
{
    None = 0,
    Ctrl = 1 << 0,
    Alt = 1 << 1,
    Shift = 1 << 2,
};
TZ_DECLARE_FLAGS(KeyModifiers, KeyModifier)
TZ_DECLARE_OPERATORS_FOR_FLAGS(KeyModifiers)

struct TzKeyEvent
{
    Key key{ Key::Unknown };
    KeyModifiers modifiers{ KeyModifier::None };
    std::string utf8;
};

#endif // TZKEYEVENT_HPP
