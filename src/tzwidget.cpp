#include <event-loop/tzwidget.hpp>
#include <event-loop/tzanchors.hpp>
#include <event-loop/tzscene.hpp>
#include <event-loop/tzgeometrychangeevent.hpp>
#include <event-loop/tzfocusevent.hpp>
#include <event-loop/tzevent.hpp>

#include "tzwidget_p.hpp"

TzWidget::TzWidget(TzWidget *parent)
    : TzObject(parent)
    , d_ptr(new TzWidgetPrivate)
{
    d_ptr->q_ptr = this;

    // Inherit scene from parent if it is already in one
    if (parent)
        d_ptr->scene = parent->d_ptr->scene;
}

TzWidget::~TzWidget()
{
    delete d_ptr->anchors;
}

// ── Geometry ──────────────────────────────────────────────────────────────

double TzWidget::x()      const { return d_ptr->geometry.x; }
double TzWidget::y()      const { return d_ptr->geometry.y; }
double TzWidget::width()  const { return d_ptr->geometry.width; }
double TzWidget::height() const { return d_ptr->geometry.height; }

TzRect  TzWidget::geometry() const { return d_ptr->geometry; }
TzPoint TzWidget::pos()      const { return d_ptr->geometry.topLeft(); }
TzSize  TzWidget::size()     const { return d_ptr->geometry.size(); }

bool TzWidget::setGeometry(const TzRect &rect)
{
    if (d_ptr->geometry == rect)
        return false;
    TzRect old = d_ptr->geometry;
    d_ptr->geometry = rect;
    geometryChanged(rect, old);
    return true;
}

void TzWidget::move(const TzPoint &pos)
{
    setGeometry({ pos.x, pos.y, d_ptr->geometry.width, d_ptr->geometry.height });
}

void TzWidget::resize(const TzSize &sz)
{
    setGeometry({ d_ptr->geometry.x, d_ptr->geometry.y, sz.width, sz.height });
}

void TzWidget::setX(double x)
{
    setGeometry({ x, d_ptr->geometry.y, d_ptr->geometry.width, d_ptr->geometry.height });
}

void TzWidget::setY(double y)
{
    setGeometry({ d_ptr->geometry.x, y, d_ptr->geometry.width, d_ptr->geometry.height });
}

void TzWidget::setWidth(double w)
{
    d_ptr->explicitWidth = true;
    setGeometry({ d_ptr->geometry.x, d_ptr->geometry.y, w, d_ptr->geometry.height });
}

void TzWidget::setHeight(double h)
{
    d_ptr->explicitHeight = true;
    setGeometry({ d_ptr->geometry.x, d_ptr->geometry.y, d_ptr->geometry.width, h });
}

void TzWidget::resetWidth()
{
    d_ptr->explicitWidth = false;
}

void TzWidget::resetHeight()
{
    d_ptr->explicitHeight = false;
}

// ── Implicit size ────────────────────────────────────────────────────────

double TzWidget::implicitWidth()  const { return d_ptr->implicitSize.width; }
double TzWidget::implicitHeight() const { return d_ptr->implicitSize.height; }
TzSize TzWidget::implicitSize()   const { return d_ptr->implicitSize; }

double TzWidget::effectiveWidth() const
{
    return d_ptr->explicitWidth ? d_ptr->geometry.width : d_ptr->implicitSize.width;
}

double TzWidget::effectiveHeight() const
{
    return d_ptr->explicitHeight ? d_ptr->geometry.height : d_ptr->implicitSize.height;
}

void TzWidget::setImplicitWidth(double w)
{
    if (d_ptr->implicitSize.width == w) return;
    d_ptr->implicitSize.width = w;
    if (d_ptr->scene) d_ptr->scene->markLayoutDirty();
}

void TzWidget::setImplicitHeight(double h)
{
    if (d_ptr->implicitSize.height == h) return;
    d_ptr->implicitSize.height = h;
    if (d_ptr->scene) d_ptr->scene->markLayoutDirty();
}

void TzWidget::setImplicitSize(double w, double h)
{
    if (d_ptr->implicitSize.width == w && d_ptr->implicitSize.height == h) return;
    d_ptr->implicitSize = { w, h };
    if (d_ptr->scene) d_ptr->scene->markLayoutDirty();
}

// ── Anchors ──────────────────────────────────────────────────────────────

TzAnchors *TzWidget::anchors()
{
    if (!d_ptr->anchors)
        d_ptr->anchors = new TzAnchors(this);
    return d_ptr->anchors;
}

bool TzWidget::resolveAnchors()
{
    if (!d_ptr->anchors)
        return false;
    return d_ptr->anchors->resolve();
}

// ── Visibility ────────────────────────────────────────────────────────────

bool TzWidget::isVisible() const { return d_ptr->visible; }

void TzWidget::setVisible(bool visible)
{
    if (d_ptr->visible == visible)
        return;
    d_ptr->visible = visible;
    update();
}

// ── Focus ─────────────────────────────────────────────────────────────────

bool TzWidget::hasFocus() const { return d_ptr->focused; }

void TzWidget::setFocus()
{
    if (d_ptr->scene)
        d_ptr->scene->setFocusedWidget(this);
}

void TzWidget::clearFocus()
{
    if (d_ptr->scene && d_ptr->scene->focusedWidget() == this)
        d_ptr->scene->setFocusedWidget(nullptr);
}

// ── Scene interaction ────────────────────────────────────────────────────

void TzWidget::update()
{
    if (d_ptr->scene)
        d_ptr->scene->markPaintDirty();
}

TzScene *TzWidget::scene() const
{
    return d_ptr->scene;
}

// ── Parent widget ────────────────────────────────────────────────────────

TzWidget *TzWidget::parentWidget() const
{
    return dynamic_cast<TzWidget *>(parent());
}

// ── Painting ─────────────────────────────────────────────────────────────

void TzWidget::paint(TzPainter * /*painter*/)
{
    // Default: transparent (no fill). Subclasses override.
}

// ── Events ───────────────────────────────────────────────────────────────

bool TzWidget::event(TzEvent *e)
{
    switch (e->type()) {
        case TzEvent::GeometryChange:
            geometryChanged(
                static_cast<TzGeometryChangeEvent *>(e)->newGeometry(),
                static_cast<TzGeometryChangeEvent *>(e)->oldGeometry());
            return true;

        case TzEvent::FocusIn:
            d_ptr->focused = true;
            update();
            return true;

        case TzEvent::FocusOut:
            d_ptr->focused = false;
            update();
            return true;

        default:
            return TzObject::event(e);
    }
}

// ── Protected helpers ────────────────────────────────────────────────────

void TzWidget::geometryChanged(const TzRect & /*newGeom*/, const TzRect & /*oldGeom*/)
{
    update();
}
