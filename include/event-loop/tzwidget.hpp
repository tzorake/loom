#ifndef TZWIDGET_HPP
#define TZWIDGET_HPP

#include <event-loop/tzobject.hpp>
#include <event-loop/tzgeometry.hpp>

#include <memory>

class TzAnchors;
class TzPainter;
class TzScene;
class TzWidgetPrivate;

class TzWidget : public TzObject
{
public:
    explicit TzWidget(TzWidget *parent = nullptr);
    virtual ~TzWidget() override;

    // ── Geometry (parent-local coordinates) ─────────────────────────────
    double x()      const;
    double y()      const;
    double width()  const;
    double height() const;

    TzRect  geometry()     const;
    TzPoint pos()          const;
    TzSize  size()         const;

    bool setGeometry(const TzRect &rect);
    void move(const TzPoint &pos);
    void resize(const TzSize &size);

    void setX(double x);
    void setY(double y);
    void setWidth(double w);   // marks width as explicit
    void setHeight(double h);  // marks height as explicit
    void resetWidth();         // reverts to implicit width
    void resetHeight();        // reverts to implicit height

    // ── Implicit (content-preferred) size ───────────────────────────────
    double implicitWidth()  const;
    double implicitHeight() const;
    TzSize implicitSize()   const;

    // ── Effective size used by layout (explicit > implicit, anchor overrides both) ──
    double effectiveWidth()  const;
    double effectiveHeight() const;

    // ── Anchors (created lazily on first call) ───────────────────────────
    TzAnchors *anchors();

    // ── Visibility ───────────────────────────────────────────────────────
    bool isVisible() const;
    void setVisible(bool visible);

    // ── Focus ────────────────────────────────────────────────────────────
    bool hasFocus() const;
    void setFocus();
    void clearFocus();

    // ── Scene interaction ────────────────────────────────────────────────
    void    update();       // marks scene paint-dirty; no-op if not in a scene
    TzScene *scene() const;

    // ── Parent widget convenience ────────────────────────────────────────
    TzWidget *parentWidget() const;

    // ── Painting ─────────────────────────────────────────────────────────
    virtual void paint(TzPainter *painter);

    // ── Event dispatch ───────────────────────────────────────────────────
    bool event(TzEvent *event) override;

    // ── Anchor resolution (called by TzScene during layout) ─────────────
    // Returns true if geometry changed.
    bool resolveAnchors();

    void setImplicitWidth(double w);
    void setImplicitHeight(double h);
    void setImplicitSize(double w, double h);

protected:
    virtual void geometryChanged(const TzRect &newGeom, const TzRect &oldGeom);

private:
    std::unique_ptr<TzWidgetPrivate> d_ptr;

    friend class TzScene;
    friend class TzAnchors;
};

#endif // TZWIDGET_HPP
