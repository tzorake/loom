#include <event-loop/tzkeyevent.hpp>

TzKeyEvent::TzKeyEvent(int type, Key key, KeyModifiers modifiers, std::string utf8)
    : TzEvent(type)
    , m_key(key)
    , m_modifiers(modifiers)
    , m_utf8(std::move(utf8))
{
}

Key TzKeyEvent::key() const
{
    return m_key;
}

KeyModifiers TzKeyEvent::modifiers() const
{
    return m_modifiers;
}

std::string TzKeyEvent::utf8() const
{
    return m_utf8;
}

TzKeyEvent *TzKeyEvent::clone() const
{
    return new TzKeyEvent(*this);
}
