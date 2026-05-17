#include <loom/tzanchors.hpp>
#include <loom/tzmargins.hpp>
#include <loom/tzrect.hpp>
#include <loom/tzwidget.hpp>

#include "tzanchors_p.hpp"

TzAnchors::TzAnchors(TzWidget *owner)
    : d_ptr(new TzAnchorsPrivate)
{
    TZ_D(TzAnchors);
    d->q_ptr = this;
    d->owner = owner;
}

TzAnchors::~TzAnchors() {}

void TzAnchors::setLeft(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->left = {target, edge, margin, true};
}

void TzAnchors::setRight(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->right = {target, edge, margin, true};
}

void TzAnchors::setTop(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->top = {target, edge, margin, true};
}

void TzAnchors::setBottom(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->bottom = {target, edge, margin, true};
}

void TzAnchors::setHCenter(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->hcenter = {target, edge, margin, true};
}

void TzAnchors::setVCenter(TzWidget *target, Edge edge, double margin)
{
    TZ_D(TzAnchors);
    d->vcenter = {target, edge, margin, true};
}

void TzAnchors::clearLeft()
{
    TZ_D(TzAnchors);
    d->left = {};
}
void TzAnchors::clearRight()
{
    TZ_D(TzAnchors);
    d->right = {};
}
void TzAnchors::clearTop()
{
    TZ_D(TzAnchors);
    d->top = {};
}
void TzAnchors::clearBottom()
{
    TZ_D(TzAnchors);
    d->bottom = {};
}
void TzAnchors::clearHCenter()
{
    TZ_D(TzAnchors);
    d->hcenter = {};
}
void TzAnchors::clearVCenter()
{
    TZ_D(TzAnchors);
    d->vcenter = {};
}

void TzAnchors::clearAll()
{
    TZ_D(TzAnchors);
    d->left = d->right = d->top = d->bottom = d->hcenter = d->vcenter = {};
}

void TzAnchors::fill(TzWidget *target, double margin)
{
    fill(target, TzMargins(margin));
}

void TzAnchors::fill(TzWidget *target, const TzMargins &margins)
{
    TZ_D(TzAnchors);
    d->left = {target, Left, margins.left, true};
    d->right = {target, Right, margins.right, true};
    d->top = {target, Top, margins.top, true};
    d->bottom = {target, Bottom, margins.bottom, true};
    d->hcenter = {};
    d->vcenter = {};
}

void TzAnchors::centerIn(TzWidget *target)
{
    TZ_D(TzAnchors);
    d->hcenter = {target, HCenter, 0.0, true};
    d->vcenter = {target, VCenter, 0.0, true};
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
    TzRect g = targetIsParent ? TzRect{0.0, 0.0, target->width(), target->height()}
                              : target->geometry();

    switch (e) {
    case TzAnchors::Left:
        return g.x;
    case TzAnchors::Right:
        return g.x + g.width;
    case TzAnchors::Top:
        return g.y;
    case TzAnchors::Bottom:
        return g.y + g.height;
    case TzAnchors::HCenter:
        return g.x + g.width / 2.0;
    case TzAnchors::VCenter:
        return g.y + g.height / 2.0;
    }
    return 0.0;
}

std::optional<TzAnchorsPrivate::Layout> TzAnchorsPrivate::computeLayout() const
{
    const bool anyH = left.set || right.set || hcenter.set;
    const bool anyV = top.set || bottom.set || vcenter.set;
    if (!anyH && !anyV)
        return std::nullopt;

    const double implW = owner->implicitWidth();
    const double implH = owner->implicitHeight();

    TzRect cur = owner->geometry();
    Layout out{cur.x, cur.y, std::nullopt, std::nullopt};

    TzWidget *ownerParent = owner->parentWidget();
    auto ev = [&](const TzAnchorLine &line) {
        return edgeValue(line.target, line.edge, line.target == ownerParent);
    };

    if (left.set && right.set) {
        double lv = ev(left) + left.margin;
        double rv = ev(right) - right.margin;
        out.x = lv;
        out.width = rv - lv;
    } else if (left.set) {
        out.x = ev(left) + left.margin;
    } else if (right.set) {
        out.x = ev(right) - right.margin - implW;
    } else if (hcenter.set) {
        out.x = ev(hcenter) + hcenter.margin - implW / 2.0;
    }

    if (top.set && bottom.set) {
        double tv = ev(top) + top.margin;
        double bv = ev(bottom) - bottom.margin;
        out.y = tv;
        out.height = bv - tv;
    } else if (top.set) {
        out.y = ev(top) + top.margin;
    } else if (bottom.set) {
        out.y = ev(bottom) - bottom.margin - implH;
    } else if (vcenter.set) {
        out.y = ev(vcenter) + vcenter.margin - implH / 2.0;
    }

    return out;
}

bool TzAnchors::resolve()
{
    TZ_D(TzAnchors);
    auto layout = d->computeLayout();
    if (!layout)
        return false;
    TzRect next = d->owner->geometry();
    next.x = layout->x;
    next.y = layout->y;
    if (layout->width.has_value())
        next.width = layout->width.value();
    if (layout->height.has_value())
        next.height = layout->height.value();
    return d->owner->setGeometry(next);
}
