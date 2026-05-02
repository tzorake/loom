#include <event-loop/tzanchors.hpp>
#include <event-loop/tzwidget.hpp>
#include <event-loop/tzmargins.hpp>
#include <event-loop/tzrect.hpp>

#include "tzanchors_p.hpp"

TzAnchors::TzAnchors(TzWidget *owner)
    : d_ptr(new TzAnchorsPrivate)
{
    TZ_D(TzAnchors);
    d->q_ptr = this;
    d->owner = owner;
}

TzAnchors::~TzAnchors()
{
}

void TzAnchors::setLeft(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->left = { target, edge, margin, true };
}

void TzAnchors::setRight(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->right = { target, edge, margin, true };
}

void TzAnchors::setTop(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->top = { target, edge, margin, true };
}

void TzAnchors::setBottom(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->bottom = { target, edge, margin, true };
}

void TzAnchors::setHCenter(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->hcenter = { target, edge, margin, true };
}

void TzAnchors::setVCenter(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->vcenter = { target, edge, margin, true };
}

void TzAnchors::clearLeft()    { TZ_D(TzAnchors); d->left    = {}; }
void TzAnchors::clearRight()   { TZ_D(TzAnchors); d->right   = {}; }
void TzAnchors::clearTop()     { TZ_D(TzAnchors); d->top     = {}; }
void TzAnchors::clearBottom()  { TZ_D(TzAnchors); d->bottom  = {}; }
void TzAnchors::clearHCenter() { TZ_D(TzAnchors); d->hcenter = {}; }
void TzAnchors::clearVCenter() { TZ_D(TzAnchors); d->vcenter = {}; }

void TzAnchors::clearAll()
{
    TZ_D(TzAnchors);
    d->left = d->right = d->top = d->bottom = d->hcenter = d->vcenter = {};
}

// ── Shorthand ─────────────────────────────────────────────────────────────

void TzAnchors::fill(TzWidget *target, double margin)
{
    fill(target, TzMargins(margin));
}

void TzAnchors::fill(TzWidget *target, const TzMargins &margins)
{
    TZ_D(TzAnchors);
    d->left = { target, Left, margins.left, true };
    d->right = { target, Right, margins.right, true };
    d->top = { target, Top, margins.top, true };
    d->bottom = { target, Bottom, margins.bottom, true };
    d->hcenter = {};
    d->vcenter = {};
}

void TzAnchors::centerIn(TzWidget *target)
{
    TZ_D(TzAnchors);
    d->hcenter = { target, HCenter, 0.0, true };
    d->vcenter = { target, VCenter, 0.0, true };
    d->left = d->right = d->top = d->bottom = {};
}

bool TzAnchors::hasLeft() const
{
    TZ_D(const TzAnchors);
    return d->left.set;
}

bool TzAnchors::hasRight() const
{
    TZ_D(const TzAnchors);
    return d->right.set;
}

bool TzAnchors::hasTop() const
{
    TZ_D(const TzAnchors);
    return d->top.set;
}

bool TzAnchors::hasBottom() const
{
    TZ_D(const TzAnchors);
    return d->bottom.set;
}

bool TzAnchors::hasHCenter() const
{
    TZ_D(const TzAnchors);
    return d->hcenter.set;
}

bool TzAnchors::hasVCenter() const
{
    TZ_D(const TzAnchors);
    return d->vcenter.set;
}

static double edgeValue(const TzWidget *target, TzAnchors::Edge e, bool targetIsParent)
{
    TzRect g = targetIsParent
        ? TzRect{ 0.0, 0.0, target->width(), target->height() }
        : target->geometry();

    switch (e) {
        case TzAnchors::Left:    return g.x;
        case TzAnchors::Right:   return g.x + g.width;
        case TzAnchors::Top:     return g.y;
        case TzAnchors::Bottom:  return g.y + g.height;
        case TzAnchors::HCenter: return g.x + g.width  / 2.0;
        case TzAnchors::VCenter: return g.y + g.height / 2.0;
    }
    return 0.0;
}

// ── Resolve ───────────────────────────────────────────────────────────────

bool TzAnchors::resolve()
{
    TZ_D(TzAnchors);

    TzRect next = d->owner->geometry();

    double effectiveW = d->owner->effectiveWidth();
    double effectiveH = d->owner->effectiveHeight();

    TzWidget *ownerParent = d->owner->parentWidget();

    auto ev = [&](const TzAnchorLine &line) {
        return edgeValue(line.target, line.edge, line.target == ownerParent);
    };

    // ── Horizontal ──────────────────────────────────────────────────────
    if (d->left.set && d->right.set) {
        double lv  = ev(d->left)  + d->left.margin;
        double rv  = ev(d->right) - d->right.margin;
        next.x     = lv;
        next.width = rv - lv;
    } else if (d->left.set) {
        next.x     = ev(d->left) + d->left.margin;
        next.width = effectiveW;
    } else if (d->right.set) {
        next.width = effectiveW;
        next.x     = ev(d->right) - d->right.margin - next.width;
    } else if (d->hcenter.set) {
        next.width = effectiveW;
        next.x     = ev(d->hcenter) + d->hcenter.margin - next.width / 2.0;
    } else {
        next.width = effectiveW;
    }

    // ── Vertical ────────────────────────────────────────────────────────
    if (d->top.set && d->bottom.set) {
        double tv   = ev(d->top)    + d->top.margin;
        double bv   = ev(d->bottom) - d->bottom.margin;
        next.y      = tv;
        next.height = bv - tv;
    } else if (d->top.set) {
        next.y      = ev(d->top) + d->top.margin;
        next.height = effectiveH;
    } else if (d->bottom.set) {
        next.height = effectiveH;
        next.y      = ev(d->bottom) - d->bottom.margin - next.height;
    } else if (d->vcenter.set) {
        next.height = effectiveH;
        next.y      = ev(d->vcenter) + d->vcenter.margin - next.height / 2.0;
    } else {
        next.height = effectiveH;
    }

    return d->owner->setGeometry(next);
}
