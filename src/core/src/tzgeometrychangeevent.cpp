#include <event-loop/tzgeometrychangeevent.hpp>

TzGeometryChangeEvent::TzGeometryChangeEvent(const TzRect &oldGeometry, const TzRect &newGeometry)
    : TzEvent(TzEvent::GeometryChange)
    , m_oldGeometry(oldGeometry)
    , m_newGeometry(newGeometry)
{
}

const TzRect &TzGeometryChangeEvent::oldGeometry() const
{
    return m_oldGeometry;
}

const TzRect &TzGeometryChangeEvent::newGeometry() const
{
    return m_newGeometry;
}

TzGeometryChangeEvent *TzGeometryChangeEvent::clone() const
{
    return new TzGeometryChangeEvent(*this);
}
