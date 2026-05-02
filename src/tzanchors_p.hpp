#ifndef TZANCHORS_P_HPP
#define TZANCHORS_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzanchors.hpp>

class TzWidget;

struct TzAnchorLine {
    TzWidget       *target{ nullptr };
    TzAnchors::Edge edge{};
    double          margin{ 0.0 };
    bool            set{ false };
};

class TzAnchorsPrivate
{
    TZ_DECLARE_PUBLIC(TzAnchors)
public:
    TzAnchors *q_ptr{ nullptr };
    TzWidget  *owner{ nullptr };

    TzAnchorLine left, right, top, bottom, hcenter, vcenter;
};

#endif // TZANCHORS_P_HPP
