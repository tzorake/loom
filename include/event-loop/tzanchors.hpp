#ifndef TZANCHORS_HPP
#define TZANCHORS_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzgeometry.hpp>

#include <memory>

class TzWidget;
class TzAnchorsPrivate;

class TzAnchors
{
    TZ_DECLARE_PRIVATE(TzAnchors)
public:
    enum Edge { Left, Right, Top, Bottom, HCenter, VCenter };

    explicit TzAnchors(TzWidget *owner);
    ~TzAnchors();

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
    void fill    (TzWidget *target, double margin = 0.0);
    void fill    (TzWidget *target, const TzMargins &margins);
    void centerIn(TzWidget *target);

    // ── Inspection ───────────────────────────────────────────────────────
    bool hasLeft()    const;
    bool hasRight()   const;
    bool hasTop()     const;
    bool hasBottom()  const;
    bool hasHCenter() const;
    bool hasVCenter() const;

    // ── Layout resolution ────────────────────────────────────────────────
    bool resolve();

private:
    std::unique_ptr<TzAnchorsPrivate> d_ptr;
};

#endif // TZANCHORS_HPP
