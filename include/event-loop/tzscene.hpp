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

    int width()  const;
    int height() const;

private:
    std::unique_ptr<TzScenePrivate> d_ptr;
};

#endif // TZSCENE_HPP
