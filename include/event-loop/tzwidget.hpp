#ifndef TZWIDGET_HPP
#define TZWIDGET_HPP

#include <event-loop/tzobject.hpp>
#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzgeometry.hpp>

class TzAnchors;
class TzPainter;
class TzScene;
class TzWidgetPrivate;

class TzWidget : public TzObject
{
    TZ_DECLARE_PRIVATE_D(d_ptr, TzWidget)
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
    void setWidth(double w);
    void setHeight(double h);
    void resetWidth();
    void resetHeight();

    // ── Implicit (content-preferred) size ───────────────────────────────
    double implicitWidth()  const;
    double implicitHeight() const;
    TzSize implicitSize()   const;

    void setImplicitWidth(double w);
    void setImplicitHeight(double h);
    void setImplicitSize(double w, double h);

    // ── Effective size used by layout ────────────────────────────────────
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
    void     update();
    TzScene *scene() const;

    // ── Parent widget convenience ────────────────────────────────────────
    TzWidget *parentWidget() const;

    // ── Painting ─────────────────────────────────────────────────────────
    virtual void paint(TzPainter *painter);

    // ── Event dispatch ───────────────────────────────────────────────────
    bool event(TzEvent *event) override;

    // ── Anchor resolution (called by TzScene during layout) ─────────────
    bool resolveAnchors();

protected:
    // Subclasses supply their own (derived) TzWidgetPrivate so no second
    // allocation is needed for the extra private data.
    explicit TzWidget(TzWidgetPrivate &d, TzWidget *parent = nullptr);

    virtual void geometryChanged(const TzRect &newGeom, const TzRect &oldGeom);

    friend class TzScene;
    friend class TzScenePrivate;
};

#endif // TZWIDGET_HPP
