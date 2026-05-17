#ifndef TZKEYEVENT_HPP
#define TZKEYEVENT_HPP

#include <loom/tzevent.hpp>
#include <loom/tzflags.hpp>

#include <string>

enum class Key {
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

enum class KeyModifier {
    None = 0,
    Ctrl = 1 << 0,
    Alt = 1 << 1,
    Shift = 1 << 2,
};
TZ_DECLARE_FLAGS(KeyModifiers, KeyModifier)
TZ_DECLARE_OPERATORS_FOR_FLAGS(KeyModifiers)

class TzKeyEvent : public TzEvent
{
public:
    TzKeyEvent(int type, Key key, KeyModifiers modifiers = KeyModifier::None, std::string utf8 = {});

    Key key() const;
    KeyModifiers modifiers() const;
    std::string utf8() const;

    TzKeyEvent *clone() const override;

private:
    Key m_key;
    KeyModifiers m_modifiers;
    std::string m_utf8;
};

#endif // TZKEYEVENT_HPP
