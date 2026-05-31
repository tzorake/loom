#ifndef TZWINDOW_HPP
#define TZWINDOW_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzcloseevent.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzobject.hpp>
#include <loom/tzpaintevent.hpp>
#include <loom/tzresizeevent.hpp>
#include <loom/tzsurface.hpp>

#include <string>

class TzRhi;
class TzRhiSwapChain;
class TzWidget;
class TzScene;
class TzWindowPrivate;

class TzWindow : public TzObject, public TzSurface
{
    TZ_DECLARE_PRIVATE_D(d_ptr, TzWindow)
public:
    explicit TzWindow(TzWindow *parent = nullptr);
    TzWindow(int width, int height, TzWindow *parent = nullptr);
    ~TzWindow() override;

    void setTitle(const std::string &title);
    void show();
    void hide();
    void close();

    TzScene *scene();
    void setRootWidget(TzWidget *root);
    TzWidget *rootWidget() const;

    int width() const;
    int height() const;

    void update();

    // Attach a GPU RHI backend to this window.  Once set, the scene's doPaint()
    // uses the GPU path instead of the software rasterizer.  Pass nullptr, nullptr
    // to revert to the software path.
    void setRhi(TzRhi *rhi, TzRhiSwapChain *swapChain);

    SurfaceType surfaceType() const override;
    TzPlatformSurface *surfaceHandle() override;
    const TzPlatformSurface *surfaceHandle() const override;

protected:
    explicit TzWindow(TzWindowPrivate &d, int width, int height, TzWindow *parent = nullptr);

    virtual void paintEvent(TzPaintEvent *event);
    virtual void closeEvent(TzCloseEvent *event);
    virtual void keyEvent(TzKeyEvent *event);
    virtual void mouseEvent(TzMouseEvent *event);
    virtual void resizeEvent(TzResizeEvent *event);
};

#endif // TZWINDOW_HPP
