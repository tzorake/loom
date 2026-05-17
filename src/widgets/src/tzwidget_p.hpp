#ifndef TZWIDGET_P_HPP
#define TZWIDGET_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzrect.hpp>
#include <loom/tzsize.hpp>
#include <loom/tzwidget.hpp>

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

    double x{0.0};
    double y{0.0};
    std::optional<double> explicitWidth;
    std::optional<double> explicitHeight;
    double implicitWidth{0.0};
    double implicitHeight{0.0};

    bool visible{true};
    bool focused{false};

    std::unique_ptr<TzAnchors> anchors;
    TzScene *scene{nullptr};
};

#endif // TZWIDGET_P_HPP
