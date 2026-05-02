#include <event-loop/tzanchors.hpp>
#include <event-loop/tzwidget.hpp>

#include "tzwidget_p.hpp"

TzAnchors::TzAnchors(TzWidget *owner)
    : m_owner(owner)
{
}

// ── Individual setters ────────────────────────────────────────────────────

void TzAnchors::setLeft(TzWidget *target, Edge edge, double margin)
{
    m_left = { target, edge, margin, true };
}

void TzAnchors::setRight(TzWidget *target, Edge edge, double margin)
{
    m_right = { target, edge, margin, true };
}

void TzAnchors::setTop(TzWidget *target, Edge edge, double margin)
{
    m_top = { target, edge, margin, true };
}

void TzAnchors::setBottom(TzWidget *target, Edge edge, double margin)
{
    m_bottom = { target, edge, margin, true };
}

void TzAnchors::setHCenter(TzWidget *target, Edge edge, double margin)
{
    m_hcenter = { target, edge, margin, true };
}

void TzAnchors::setVCenter(TzWidget *target, Edge edge, double margin)
{
    m_vcenter = { target, edge, margin, true };
}

// ── Clear ──────────────────────────────────────────────────────────────────

void TzAnchors::clearLeft()    { m_left    = {}; }
void TzAnchors::clearRight()   { m_right   = {}; }
void TzAnchors::clearTop()     { m_top     = {}; }
void TzAnchors::clearBottom()  { m_bottom  = {}; }
void TzAnchors::clearHCenter() { m_hcenter = {}; }
void TzAnchors::clearVCenter() { m_vcenter = {}; }

void TzAnchors::clearAll()
{
    m_left = m_right = m_top = m_bottom = m_hcenter = m_vcenter = {};
}

// ── Shorthand ─────────────────────────────────────────────────────────────

void TzAnchors::fill(TzWidget *target, double margin)
{
    fill(target, TzMargins(margin));
}

void TzAnchors::fill(TzWidget *target, const TzMargins &margins)
{
    m_left   = { target, Left,   margins.left,   true };
    m_right  = { target, Right,  margins.right,  true };
    m_top    = { target, Top,    margins.top,    true };
    m_bottom = { target, Bottom, margins.bottom, true };
    m_hcenter = {};
    m_vcenter = {};
}

void TzAnchors::centerIn(TzWidget *target)
{
    m_hcenter = { target, HCenter, 0.0, true };
    m_vcenter = { target, VCenter, 0.0, true };
    m_left = m_right = m_top = m_bottom = {};
}

// ── Inspection ────────────────────────────────────────────────────────────

bool TzAnchors::hasLeft()    const { return m_left.set;    }
bool TzAnchors::hasRight()   const { return m_right.set;   }
bool TzAnchors::hasTop()     const { return m_top.set;     }
bool TzAnchors::hasBottom()  const { return m_bottom.set;  }
bool TzAnchors::hasHCenter() const { return m_hcenter.set; }
bool TzAnchors::hasVCenter() const { return m_vcenter.set; }

// ── Edge value resolver ───────────────────────────────────────────────────
//
// Geometries are stored in parent-local coordinates.  When a child anchors to
// its own parent the target rect must be expressed in the parent's *own* local
// space (i.e. origin at 0,0 with the parent's dimensions), not in the
// grandparent space that geometry() returns.  For siblings the geometry() is
// already in the same space as the owner, so no adjustment is needed.

double TzAnchors::edgeValue(const TzWidget *target, Edge e, bool targetIsParent)
{
    TzRect g = targetIsParent
        ? TzRect{ 0.0, 0.0, target->width(), target->height() }
        : target->geometry();

    switch (e) {
        case Left:    return g.x;
        case Right:   return g.x + g.width;
        case Top:     return g.y;
        case Bottom:  return g.y + g.height;
        case HCenter: return g.x + g.width  / 2.0;
        case VCenter: return g.y + g.height / 2.0;
    }
    return 0.0;
}

// ── Resolve ───────────────────────────────────────────────────────────────

bool TzAnchors::resolve()
{
    TzRect next = m_owner->geometry();

    double effectiveW = m_owner->effectiveWidth();
    double effectiveH = m_owner->effectiveHeight();

    TzWidget *ownerParent = m_owner->parentWidget();

    auto ev = [&](const AnchorLine &line) {
        return edgeValue(line.target, line.edge, line.target == ownerParent);
    };

    // ── Horizontal ──────────────────────────────────────────────────────
    if (m_left.set && m_right.set) {
        double lv  = ev(m_left)  + m_left.margin;
        double rv  = ev(m_right) - m_right.margin;
        next.x     = lv;
        next.width = rv - lv;
    } else if (m_left.set) {
        next.x     = ev(m_left) + m_left.margin;
        next.width = effectiveW;
    } else if (m_right.set) {
        next.width = effectiveW;
        next.x     = ev(m_right) - m_right.margin - next.width;
    } else if (m_hcenter.set) {
        next.width = effectiveW;
        next.x     = ev(m_hcenter) + m_hcenter.margin - next.width / 2.0;
    } else {
        next.width = effectiveW;
    }

    // ── Vertical ────────────────────────────────────────────────────────
    if (m_top.set && m_bottom.set) {
        double tv   = ev(m_top)    + m_top.margin;
        double bv   = ev(m_bottom) - m_bottom.margin;
        next.y      = tv;
        next.height = bv - tv;
    } else if (m_top.set) {
        next.y      = ev(m_top) + m_top.margin;
        next.height = effectiveH;
    } else if (m_bottom.set) {
        next.height = effectiveH;
        next.y      = ev(m_bottom) - m_bottom.margin - next.height;
    } else if (m_vcenter.set) {
        next.height = effectiveH;
        next.y      = ev(m_vcenter) + m_vcenter.margin - next.height / 2.0;
    } else {
        next.height = effectiveH;
    }

    return m_owner->setGeometry(next);
}
