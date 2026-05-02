#include <event-loop/tzwidget.hpp>
#include <event-loop/tzanchors.hpp>
#include <event-loop/tzscene.hpp>
#include <event-loop/tzgeometrychangeevent.hpp>
#include <event-loop/tzfocusevent.hpp>
#include <event-loop/tzpaintevent.hpp>
#include <event-loop/tzevent.hpp>
#include <event-loop/tzrect.hpp>
#include <event-loop/tzpoint.hpp>
#include <event-loop/tzsize.hpp>
#include <event-loop/tznumeric.hpp>

#include "tzwidget_p.hpp"


double TzWidgetPrivate::effectiveWidth() const
{
    return explicitWidth.value_or(implicitWidth);
}

double TzWidgetPrivate::effectiveHeight() const
{
    return explicitHeight.value_or(implicitHeight);
}

TzRect TzWidgetPrivate::effectiveGeometry() const
{
    return { x, y, effectiveWidth(), effectiveHeight() };
}

void TzWidgetPrivate::resetWidth() 
{
    explicitWidth = std::nullopt;
}

void TzWidgetPrivate::resetHeight() 
{
    explicitHeight = std::nullopt;
}

bool TzWidgetPrivate::resolveAnchors()
{
    return anchors ? anchors->resolve() : false;
}

void TzWidgetPrivate::clearFocus()
{
    if (scene && scene->focusedWidget() == q_ptr)
        scene->setFocusedWidget(nullptr);
}

TzWidget::TzWidget(TzWidget *parent)
    : TzWidget(*new TzWidgetPrivate, parent)
{
}

TzWidget::TzWidget(TzWidgetPrivate &dd, TzWidget *parent)
    : TzObject(dd, parent)
{
    TZ_D(TzWidget);
    d->q_ptr = this;
    if (parent)
        d->scene = static_cast<TzWidgetPrivate *>(parent->d_ptr)->scene;
}

TzWidgetPrivate::~TzWidgetPrivate()
{
}

TzWidget::~TzWidget()
{
}

void TzWidget::setX(double x)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(d->x, x))
        return;
    TzRect old = d->effectiveGeometry();
    d->x = x;
    geometryChanged(d->effectiveGeometry(), old);
}

double TzWidget::x() const
{
    TZ_D(const TzWidget);
    return d->x;
}

void TzWidget::setY(double y)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(d->y, y))
        return;
    TzRect old = d->effectiveGeometry();
    d->y = y;
    geometryChanged(d->effectiveGeometry(), old);
}

double TzWidget::y() const
{
    TZ_D(const TzWidget);
    return d->y;
}

void TzWidget::setWidth(double width)
{
    TZ_D(TzWidget);
    if (d->explicitWidth.has_value() && 
        tzFuzzyCompare(d->explicitWidth.value(), width))
        return;
    TzRect old = d->effectiveGeometry();
    d->explicitWidth = width;
    geometryChanged(d->effectiveGeometry(), old);
}

double TzWidget::width() const
{
    TZ_D(const TzWidget);
    return d->effectiveWidth();
}

void TzWidget::setHeight(double height)
{
    TZ_D(TzWidget);
    if (d->explicitHeight.has_value() &&
        tzFuzzyCompare(d->explicitHeight.value(), height))
        return;
    TzRect old = d->effectiveGeometry();
    d->explicitHeight = height;
    geometryChanged(d->effectiveGeometry(), old);
}

double TzWidget::height() const
{
    TZ_D(const TzWidget);
    return d->effectiveHeight();
}

TzRect TzWidget::geometry() const
{
    TZ_D(const TzWidget);
    return d->effectiveGeometry();
}

TzPoint TzWidget::position() const
{
    TZ_D(const TzWidget);
    return d->effectiveGeometry().topLeft();
}

TzSize TzWidget::size() const
{
    TZ_D(const TzWidget);
    return d->effectiveGeometry().size();
}

bool TzWidget::setGeometry(double x, double y, double width, double height)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(x, d->x) && 
        tzFuzzyCompare(y, d->y) &&
        d->explicitWidth.has_value() && tzFuzzyCompare(d->explicitWidth.value(), width) && 
        d->explicitHeight.has_value() && tzFuzzyCompare(d->explicitHeight.value(), height))
        return false;
    TzRect old = d->effectiveGeometry();
    d->x = x;
    d->y = y;
    d->explicitWidth = width;
    d->explicitHeight = height;
    geometryChanged(d->effectiveGeometry(), old);
    return true;
}

bool TzWidget::setGeometry(const TzRect &rect)
{
    return setGeometry(rect.x, rect.y, rect.width, rect.height);
}

void TzWidget::move(const TzPoint &pos)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(d->x, pos.x) &&
        tzFuzzyCompare(d->y, pos.y))
        return;
    TzRect old = d->effectiveGeometry();
    d->x = pos.x;
    d->y = pos.y;
    geometryChanged(d->effectiveGeometry(), old);
}

void TzWidget::resize(const TzSize &size)
{
    TZ_D(TzWidget);
    setGeometry(d->x, d->y, size.width, size.height);
}

void TzWidget::setImplicitWidth(double width)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(d->implicitWidth, width))
        return;
    d->implicitWidth = width;
    if (d->scene)
        d->scene->markLayoutDirty();
}

double TzWidget::implicitWidth() const
{
    TZ_D(const TzWidget);
    return d->implicitWidth;
}

void TzWidget::setImplicitHeight(double height)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(d->implicitHeight, height))
        return;
    d->implicitHeight = height;
    if (d->scene)
        d->scene->markLayoutDirty();
}

double TzWidget::implicitHeight() const
{
    TZ_D(const TzWidget);
    return d->implicitHeight;
}

TzSize TzWidget::implicitSize() const
{
    TZ_D(const TzWidget);
    return { d->effectiveWidth(), d->effectiveHeight() };
}

double TzWidget::effectiveWidth() const
{
    TZ_D(const TzWidget);
    return d->effectiveWidth();
}

double TzWidget::effectiveHeight() const
{
    TZ_D(const TzWidget);
    return d->effectiveHeight();
}

void TzWidget::setImplicitSize(double width, double height)
{
    TZ_D(TzWidget);
    if (tzFuzzyCompare(d->implicitWidth, width) && 
        tzFuzzyCompare(d->implicitHeight, height))
        return;
    d->implicitWidth = width;
    d->implicitHeight = height;
    if (d->scene)
        d->scene->markLayoutDirty();
}

TzAnchors *TzWidget::anchors()
{
    TZ_D(TzWidget);
    if (!d->anchors)
        d->anchors.reset(new TzAnchors(this));
    return d->anchors.get();
}

void TzWidget::setVisible(bool visible)
{
    TZ_D(TzWidget);
    if (d->visible == visible)
        return;
    d->visible = visible;
    update();
}

bool TzWidget::isVisible() const
{
    TZ_D(const TzWidget);
    return d->visible;
}

void TzWidget::setFocus()
{
    TZ_D(TzWidget);
    if (d->scene)
        d->scene->setFocusedWidget(this);
}

bool TzWidget::hasFocus() const
{
    TZ_D(const TzWidget);
    return d->focused;
}

void TzWidget::update()
{
    TZ_D(TzWidget);
    if (d->scene)
        d->scene->markPaintDirty();
}

TzScene *TzWidget::scene() const
{
    TZ_D(const TzWidget);
    return d->scene;
}

TzWidget *TzWidget::parentWidget() const
{
    return dynamic_cast<TzWidget *>(parent());
}

void TzWidget::paint(TzPainter * painter)
{
    (void)painter;
}

bool TzWidget::event(TzEvent *e)
{
    TZ_D(TzWidget);
    switch (e->type()) {
        case TzEvent::Paint:
            paint(static_cast<TzPaintEvent *>(e)->painter());
            return true;

        case TzEvent::GeometryChange:
            geometryChanged(
                static_cast<TzGeometryChangeEvent *>(e)->newGeometry(),
                static_cast<TzGeometryChangeEvent *>(e)->oldGeometry());
            return true;

        case TzEvent::FocusIn:
            d->focused = true;
            update();
            return true;

        case TzEvent::FocusOut:
            d->focused = false;
            update();
            return true;

        default:
            return TzObject::event(e);
    }
}

void TzWidget::geometryChanged(const TzRect &newGeom, const TzRect &oldGeom)
{
    (void)newGeom; (void)oldGeom;
    update();
}
