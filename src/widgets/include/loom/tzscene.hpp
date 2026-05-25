#ifndef TZSCENE_HPP
#define TZSCENE_HPP

#include <loom/tzclasshelpermacros.hpp>

#include <memory>

class TzCloseEvent;
class TzKeyEvent;
class TzMouseEvent;
class TzResizeEvent;
class TzSurface;
class TzWidget;
class TzScenePrivate;

class TzScene
{
    TZ_DECLARE_PRIVATE(TzScene)
public:
    explicit TzScene(TzSurface *surface);
    ~TzScene();

    TzWidget *root() const;
    void setRoot(TzWidget *root);

    void doLayout();

    void doPaint();
    void markPaintDirty();
    bool isPaintDirty() const;
    void markLayoutDirty();

    TzWidget *focusedWidget() const;
    void setFocusedWidget(TzWidget *widget);

    TzWidget *widgetAt(double x, double y) const;

    int width() const;
    int height() const;

    void dispatchCloseEvent(TzCloseEvent *event);
    void dispatchKeyEvent(TzKeyEvent *event);
    void dispatchMouseEvent(TzMouseEvent *event);
    void dispatchResizeEvent(TzResizeEvent *event);

private:
    std::unique_ptr<TzScenePrivate> d_ptr;
};

#endif // TZSCENE_HPP
