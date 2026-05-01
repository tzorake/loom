#include <event-loop/tzevent.hpp>

TzEvent::TzEvent(int type)
    : m_type(type)
{
}

TzEvent::~TzEvent() = default;

int TzEvent::type() const
{
    return m_type;
}
