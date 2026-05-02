#ifndef TZSCENE_HPP
#define TZSCENE_HPP

#include <event-loop/tzgeometry.hpp>

#include <cstdint>
#include <vector>

class TzAbstractWindow;
class TzKeyEvent;
class TzMouseEvent;
class TzWidget;

// TzScene bridges a TzWidget tree to a TzAbstractWindow.
//
// Responsibilities:
//   - Drives layout (anchor resolution) on window resize or when dirty
//   - Drives painting (pixel-buffer rendering) via doPaint()
//   - Routes key events to the focused widget
//   - Hit-tests and routes mouse events to the widget under the cursor
//
// The scene takes over the window's callbacks; do not set them separately
// while a scene is attached.
class TzScene
{
public:
    explicit TzScene(TzAbstractWindow *window);
    ~TzScene();

    // ── Root widget ───────────────────────────────────────────────────────
    TzWidget *root() const;
    // Takes ownership semantics: the root's parent is set to nullptr and its
    // scene pointer is registered for the entire subtree.
    void setRoot(TzWidget *root);

    // ── Layout ────────────────────────────────────────────────────────────
    // Resolves all anchor geometry for the widget tree top-down.
    // Automatically called on window resize.
    void doLayout();

    // ── Painting ──────────────────────────────────────────────────────────
    // Renders the widget tree into the pixel buffer and calls window->render().
    void doPaint();

    // Mark the paint buffer as needing a repaint (called by TzWidget::update()).
    void markPaintDirty();
    bool isPaintDirty() const;

    // Mark layout as dirty so doLayout() runs on the next doPaint() call.
    void markLayoutDirty();

    // ── Focus ─────────────────────────────────────────────────────────────
    TzWidget *focusedWidget() const;
    void      setFocusedWidget(TzWidget *widget);

    // ── Hit-testing ───────────────────────────────────────────────────────
    // Returns the deepest visible widget that contains (x, y) in window coords.
    TzWidget *widgetAt(double x, double y) const;

    // ── Window size ───────────────────────────────────────────────────────
    int width()  const;
    int height() const;

private:
    TzAbstractWindow      *m_window;
    TzWidget              *m_root{ nullptr };
    TzWidget              *m_focusedWidget{ nullptr };
    std::vector<uint32_t>  m_pixels;
    int                    m_width{ 0 };
    int                    m_height{ 0 };
    bool                   m_paintDirty{ true };
    bool                   m_layoutDirty{ false };

    // Window callback handlers
    void onResize(int w, int h);
    void onKey(TzKeyEvent *event);
    void onMouse(TzMouseEvent *event);

    // Recursive layout and paint helpers
    void layoutWidget(TzWidget *w, int passesLeft);
    void paintWidget (TzWidget *w, double parentAbsX, double parentAbsY,
                      const TzRect &parentClip);

    // Walk widget subtree and (un)register scene pointer
    void registerSubtree  (TzWidget *w);
    void unregisterSubtree(TzWidget *w);

    // Recursive hit-test helper
    TzWidget *widgetAtHelper(TzWidget *w,
                              double x, double y,
                              double parentAbsX, double parentAbsY) const;
};

#endif // TZSCENE_HPP
