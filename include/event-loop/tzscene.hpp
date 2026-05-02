#ifndef TZSCENE_HPP
#define TZSCENE_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <memory>

class TzAbstractWindow;
class TzWidget;
class TzScenePrivate;

class TzScene
{
    TZ_DECLARE_PRIVATE(TzScene)
public:
    explicit TzScene(TzAbstractWindow *window);
    ~TzScene();

    // ── Root widget ───────────────────────────────────────────────────────
    TzWidget *root() const;
    void      setRoot(TzWidget *root);

    // ── Layout ────────────────────────────────────────────────────────────
    void doLayout();

    // ── Painting ──────────────────────────────────────────────────────────
    void doPaint();
    void markPaintDirty();
    bool isPaintDirty() const;
    void markLayoutDirty();

    // ── Focus ─────────────────────────────────────────────────────────────
    TzWidget *focusedWidget() const;
    void      setFocusedWidget(TzWidget *widget);

    // ── Hit-testing ───────────────────────────────────────────────────────
    TzWidget *widgetAt(double x, double y) const;

    // ── Window size ───────────────────────────────────────────────────────
    int width()  const;
    int height() const;

private:
    std::unique_ptr<TzScenePrivate> d_ptr;
};

#endif // TZSCENE_HPP
