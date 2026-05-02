#ifndef TZANCHORS_HPP
#define TZANCHORS_HPP

#include <event-loop/tzgeometry.hpp>

class TzWidget;

class TzAnchors
{
public:
    enum Edge { Left, Right, Top, Bottom, HCenter, VCenter };

    explicit TzAnchors(TzWidget *owner);

    // ── Individual edge bindings ─────────────────────────────────────────
    void setLeft   (TzWidget *target, Edge targetEdge, double margin = 0.0);
    void setRight  (TzWidget *target, Edge targetEdge, double margin = 0.0);
    void setTop    (TzWidget *target, Edge targetEdge, double margin = 0.0);
    void setBottom (TzWidget *target, Edge targetEdge, double margin = 0.0);
    void setHCenter(TzWidget *target, Edge targetEdge, double margin = 0.0);
    void setVCenter(TzWidget *target, Edge targetEdge, double margin = 0.0);

    // ── Clear individual bindings ────────────────────────────────────────
    void clearLeft();
    void clearRight();
    void clearTop();
    void clearBottom();
    void clearHCenter();
    void clearVCenter();
    void clearAll();

    // ── Shorthand setters ────────────────────────────────────────────────
    // fill: binds all four edges to target with uniform or per-edge margins
    void fill    (TzWidget *target, double margin = 0.0);
    void fill    (TzWidget *target, const TzMargins &margins);

    // centerIn: binds hcenter+vcenter to target's center
    void centerIn(TzWidget *target);

    // ── Inspection ───────────────────────────────────────────────────────
    bool hasLeft()    const;
    bool hasRight()   const;
    bool hasTop()     const;
    bool hasBottom()  const;
    bool hasHCenter() const;
    bool hasVCenter() const;

    // ── Layout resolution ────────────────────────────────────────────────
    // Recomputes owner geometry from active bindings.
    // Returns true if geometry changed.
    bool resolve();

private:
    struct AnchorLine {
        TzWidget *target{ nullptr };
        Edge      edge{};
        double    margin{ 0.0 };
        bool      set{ false };
    };

    TzWidget   *m_owner;
    AnchorLine  m_left, m_right, m_top, m_bottom, m_hcenter, m_vcenter;

    static double edgeValue(const TzWidget *target, Edge e, bool targetIsParent);
};

#endif // TZANCHORS_HPP
