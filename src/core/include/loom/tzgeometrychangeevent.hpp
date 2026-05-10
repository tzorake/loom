#ifndef TZGEOMETRYCHANGEEVENT_HPP
#define TZGEOMETRYCHANGEEVENT_HPP

#include <loom/tzevent.hpp>
#include <loom/tzrect.hpp>

class TzGeometryChangeEvent : public TzEvent
{
public:
    TzGeometryChangeEvent(const TzRect &oldGeometry, const TzRect &newGeometry);

    const TzRect &oldGeometry() const;
    const TzRect &newGeometry() const;

    TzGeometryChangeEvent *clone() const override;

private:
    TzRect m_oldGeometry;
    TzRect m_newGeometry;
};

#endif // TZGEOMETRYCHANGEEVENT_HPP
