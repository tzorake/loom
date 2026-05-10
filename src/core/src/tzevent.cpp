#include <loom/tzevent.hpp>

TzEvent::TzEvent(int type)
    : m_type(type)
{
}

TzEvent::~TzEvent() = default;

int TzEvent::type() const
{
    return m_type;
}

void TzEvent::setAccepted(bool accepted)
{
    m_accepted = accepted;
}

bool TzEvent::isAccepted() const
{
    return m_accepted;
}

void TzEvent::accept()
{
    m_accepted = true;
}

void TzEvent::ignore()
{
    m_accepted = false;
}

TzEvent *TzEvent::clone() const
{
    return new TzEvent(*this);
}
