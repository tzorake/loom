#ifndef TZWIDGET_P_HPP
#define TZWIDGET_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzwidget.hpp>
#include <event-loop/tzrect.hpp>
#include <event-loop/tzsize.hpp>

#include "tzobject_p.hpp"

#include <memory>
#include <optional>

class TzScene;
class TzAnchors;

class TzWidgetPrivate : public TzObjectPrivate
{
    TZ_DECLARE_PUBLIC(TzWidget)
public:
    virtual ~TzWidgetPrivate() override;
    
    double effectiveWidth() const;
    double effectiveHeight() const;
    TzRect effectiveGeometry() const;
    
    void resetWidth();
    void resetHeight();
    bool resolveAnchors();
    void clearFocus();

    double x{ 0.0 };
    double y{ 0.0 };
    std::optional<double> explicitWidth;
    std::optional<double> explicitHeight;
    double implicitWidth{ 0.0 };
    double implicitHeight{ 0.0 };

    bool visible{ true };
    bool focused{ false };

    std::unique_ptr<TzAnchors> anchors;
    TzScene *scene{ nullptr };
};

#endif // TZWIDGET_P_HPP
