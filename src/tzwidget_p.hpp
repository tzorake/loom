#ifndef TZWIDGET_P_HPP
#define TZWIDGET_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzwidget.hpp>

#include "tzobject_p.hpp"

#include <memory>

class TzScene;
class TzAnchors;

class TzWidgetPrivate : public TzObjectPrivate
{
    TZ_DECLARE_PUBLIC(TzWidget)
public:
    virtual ~TzWidgetPrivate() override;

    TzRect geometry{};
    TzSize implicitSize{};

    bool explicitWidth{ false };
    bool explicitHeight{ false };
    bool visible{ true };
    bool focused{ false };

    std::unique_ptr<TzAnchors> anchors;
    TzScene *scene{ nullptr };
};

#endif // TZWIDGET_P_HPP
