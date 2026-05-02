#include <event-loop/tzwidget.hpp>
#include <event-loop/tzanchors.hpp>
#include <event-loop/tzscene.hpp>
#include <event-loop/tzgeometrychangeevent.hpp>
#include <event-loop/tzfocusevent.hpp>
#include <event-loop/tzpaintevent.hpp>
#include <event-loop/tzevent.hpp>

#include "tzwidget_p.hpp"

// ── Constructors ──────────────────────────────────────────────────────────

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
    setGeometry({ x, d->geometry.y, d->geometry.width, d->geometry.height });
}

double TzWidget::x() const
{
    TZ_D(const TzWidget);
    return d->geometry.x;
}

void TzWidget::setY(double y)
{
    TZ_D(TzWidget);
    setGeometry({ d->geometry.x, y, d->geometry.width, d->geometry.height });
}

double TzWidget::y() const
{
    TZ_D(const TzWidget);
    return d->geometry.y;
}

void TzWidget::setWidth(double w)
{
    TZ_D(TzWidget);
    d->explicitWidth = true;
    setGeometry({ d->geometry.x, d->geometry.y, w, d->geometry.height });
}

double TzWidget::width() const
{
    TZ_D(const TzWidget);
    return d->geometry.width;
}

void TzWidget::setHeight(double h)
{
    TZ_D(TzWidget);
    d->explicitHeight = true;
    setGeometry({ d->geometry.x, d->geometry.y, d->geometry.width, h });
}

double TzWidget::height() const
{
    TZ_D(const TzWidget);
    return d->geometry.height;
}

TzRect TzWidget::geometry() const
{
    TZ_D(const TzWidget);
    return d->geometry;
}

TzPoint TzWidget::position() const
{
    TZ_D(const TzWidget);
    return d->geometry.topLeft();
}

TzSize TzWidget::size() const
{
    TZ_D(const TzWidget);
    return d->geometry.size();
}

bool TzWidget::setGeometry(const TzRect &rect)
{
    TZ_D(TzWidget);
    if (d->geometry == rect)
        return false;
    TzRect old = d->geometry;
    d->geometry = rect;
    geometryChanged(rect, old);
    return true;
}

void TzWidget::move(const TzPoint &pos)
{
    TZ_D(TzWidget);
    setGeometry({ pos.x, pos.y, d->geometry.width, d->geometry.height });
}

void TzWidget::resize(const TzSize &sz)
{
    TZ_D(TzWidget);
    setGeometry({ d->geometry.x, d->geometry.y, sz.width, sz.height });
}

void TzWidget::resetWidth() 
{
    TZ_D(TzWidget);
    d->explicitWidth = false;
}

void TzWidget::resetHeight() 
{
    TZ_D(TzWidget);
    d->explicitHeight = false;
}

void TzWidget::setImplicitWidth(double w)
{
    TZ_D(TzWidget);
    if (d->implicitSize.width == w)
        return;
    d->implicitSize.width = w;
    if (d->scene)
        d->scene->markLayoutDirty();
}

double TzWidget::implicitWidth() const
{
    TZ_D(const TzWidget);
    return d->implicitSize.width;
}

void TzWidget::setImplicitHeight(double h)
{
    TZ_D(TzWidget);
    if (d->implicitSize.height == h)
        return;
    d->implicitSize.height = h;
    if (d->scene)
        d->scene->markLayoutDirty();
}

double TzWidget::implicitHeight() const
{
    TZ_D(const TzWidget);
    return d->implicitSize.height;
}

TzSize TzWidget::implicitSize() const
{
    TZ_D(const TzWidget);
    return d->implicitSize;
}

double TzWidget::effectiveWidth() const
{
    TZ_D(const TzWidget);
    return d->explicitWidth ? d->geometry.width : d->implicitSize.width;
}

double TzWidget::effectiveHeight() const
{
    TZ_D(const TzWidget);
    return d->explicitHeight ? d->geometry.height : d->implicitSize.height;
}

void TzWidget::setImplicitSize(double w, double h)
{
    TZ_D(TzWidget);
    if (d->implicitSize.width == w && d->implicitSize.height == h)
        return;
    d->implicitSize = { w, h };
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

bool TzWidget::resolveAnchors()
{
    TZ_D(TzWidget);
    return d->anchors ? d->anchors->resolve() : false;
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

void TzWidget::clearFocus()
{
    TZ_D(TzWidget);
    if (d->scene && d->scene->focusedWidget() == this)
        d->scene->setFocusedWidget(nullptr);
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
