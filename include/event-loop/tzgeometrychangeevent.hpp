#ifndef TZGEOMETRYCHANGEEVENT_HPP
#define TZGEOMETRYCHANGEEVENT_HPP

#include <event-loop/tzevent.hpp>
#include <event-loop/tzgeometry.hpp>

class TzGeometryChangeEvent : public TzEvent
{
public:
    TzGeometryChangeEvent(const TzRect &oldGeometry, const TzRect &newGeometry)
        : TzEvent(TzEvent::GeometryChange)
        , m_oldGeometry(oldGeometry)
        , m_newGeometry(newGeometry)
    {}

    const TzRect &oldGeometry() const { return m_oldGeometry; }
    const TzRect &newGeometry() const { return m_newGeometry; }

    TzGeometryChangeEvent *clone() const override { return new TzGeometryChangeEvent(*this); }

private:
    TzRect m_oldGeometry;
    TzRect m_newGeometry;
};

#endif // TZGEOMETRYCHANGEEVENT_HPP
