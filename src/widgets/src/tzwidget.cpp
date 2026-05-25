#include <loom/tzanchors.hpp>
#include <loom/tzevent.hpp>
#include <loom/tzfocusevent.hpp>
#include <loom/tzgeometrychangeevent.hpp>
#include <loom/tznumeric.hpp>
#include <loom/tzpaintevent.hpp>
#include <loom/tzpoint.hpp>
#include <loom/tzrect.hpp>
#include <loom/tzscene.hpp>
#include <loom/tzsize.hpp>
#include <loom/tzwidget.hpp>

#include "tzanchors_p.hpp"
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
    return {x, y, effectiveWidth(), effectiveHeight()};
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
    if (!anchors)
        return false;

    std::optional<TzAnchorsPrivate::Layout> layout
        = static_cast<TzAnchorsPrivate *>(anchors->d_ptr.get())->computeLayout();
    if (!layout)
        return false;

    TzRect old = effectiveGeometry();
    x = layout->x;
    y = layout->y;
    if (layout->width)
        explicitWidth = layout->width;

    if (layout->height)
        explicitHeight = layout->height;

    TzRect newEff = effectiveGeometry();
    if (old == newEff)
        return false;

    TZ_Q(TzWidget);
    q->geometryChanged(newEff, old);

    return true;
}

void TzWidgetPrivate::clearFocus()
{
    if (scene && scene->focusedWidget() == q_ptr)
        scene->setFocusedWidget(nullptr);
}

TzWidget::TzWidget(TzWidget *parent)
    : TzWidget(*new TzWidgetPrivate, parent)
{}

TzWidget::TzWidget(TzWidgetPrivate &dd, TzWidget *parent)
    : TzObject(dd, parent)
{
    TZ_D(TzWidget);
    d->q_ptr = this;
    if (parent)
        d->scene = static_cast<TzWidgetPrivate *>(parent->d_ptr.get())->scene;
}

TzWidgetPrivate::~TzWidgetPrivate() {}

TzWidget::~TzWidget() {}

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
    if (d->explicitWidth.has_value() && tzFuzzyCompare(d->explicitWidth.value(), width))
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
    if (d->explicitHeight.has_value() && tzFuzzyCompare(d->explicitHeight.value(), height))
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
    TzRect next{x, y, width, height};
    if (d->effectiveGeometry() == next)
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
    if (tzFuzzyCompare(d->x, pos.x) && tzFuzzyCompare(d->y, pos.y))
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
    return {d->implicitWidth, d->implicitHeight};
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
    if (tzFuzzyCompare(d->implicitWidth, width) && tzFuzzyCompare(d->implicitHeight, height))
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

TzPoint TzWidget::mapToGlobal(const TzPoint &local) const
{
    double ax = local.x;
    double ay = local.y;
    for (const TzWidget *p = this; p; p = p->parentWidget()) {
        ax += p->x();
        ay += p->y();
    }
    return {ax, ay};
}

TzPoint TzWidget::mapFromGlobal(const TzPoint &global) const
{
    TzPoint origin = mapToGlobal({0.0, 0.0});
    return {global.x - origin.x, global.y - origin.y};
}

void TzWidget::paint(TzPainter *painter)
{
    (void) painter;
}

bool TzWidget::event(TzEvent *e)
{
    TZ_D(TzWidget);
    switch (e->type()) {
    case TzEvent::Paint:
        paint(static_cast<TzPaintEvent *>(e)->painter());
        return true;

    case TzEvent::GeometryChange:
        geometryChanged(static_cast<TzGeometryChangeEvent *>(e)->newGeometry(),
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
    (void) newGeom;
    (void) oldGeom;
    update();
}
