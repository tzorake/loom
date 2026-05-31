#include "tzplatformwindow.hpp"
#include <loom/tzcloseevent.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzpaintevent.hpp>
#include <loom/tzpainter.hpp>
#include <loom/tzplatformsurface.hpp>
#include <loom/tzpoint.hpp>
#include <loom/tzrect.hpp>
#include <loom/tzresizeevent.hpp>
#include <loom/tzscene.hpp>
#include <loom/tzsurface.hpp>
#include <loom/tztimer.hpp>
#include <loom/tzwidget.hpp>
#include <loom/tzwindow.hpp>

#include "tzwindow_p.hpp"

#include <chrono>

TzWindowPrivate::~TzWindowPrivate()
{
    delete paintTimer;
    // Clear all platform callbacks before tearing down the scene and platform
    // window so that no lambda can fire against a half-destroyed object graph.
    if (platformWindow) {
        platformWindow->setCloseCallback({});
        platformWindow->setResizeCallback({});
        platformWindow->setKeyCallback({});
        platformWindow->setMouseCallback({});
    }
    delete scene;
    delete platformWindow;
}

TzWindow::TzWindow(TzWindow *parent)
    : TzWindow(*new TzWindowPrivate, 400, 300, parent)
{}

TzWindow::TzWindow(int width, int height, TzWindow *parent)
    : TzWindow(*new TzWindowPrivate, width, height, parent)
{}

TzWindow::TzWindow(TzWindowPrivate &dd, int width, int height, TzWindow *parent)
    : TzObject(dd, parent)
{
    TZ_D(TzWindow);
    d->q_ptr = this;

    d->platformWindow = tzCreatePlatformWindow(width, height);
    d->platformWindow->setSurface(this);

    d->scene = new TzScene(this);

    d->platformWindow->setCloseCallback([this] {
        TzCloseEvent e;
        closeEvent(&e);
    });
    d->platformWindow->setResizeCallback([this](int w, int h) {
        TzResizeEvent e(w, h);
        resizeEvent(&e);
    });
    d->platformWindow->setKeyCallback([this](TzKeyEvent *e) {
        keyEvent(e);
    });
    d->platformWindow->setMouseCallback([this](TzMouseEvent *e) {
        mouseEvent(e);
    });

    d->paintTimer = TzTimer::repeat(std::chrono::milliseconds(16), [this] {
        TZ_D(TzWindow);
        // Scene (widget) paint path
        if (d->scene->isPaintDirty())
            d->scene->doPaint();

        // Custom paint path
        if (d->paintDirty) {
            // Clear the flag before calling paintEvent so that any update()
            // call made from within paintEvent (e.g. for continuous animation)
            // survives and is not overwritten by a post-call clear.
            d->paintDirty = false;
            const int w = d->scene->width();
            const int h = d->scene->height();
            if (w > 0 && h > 0) {
                d->pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
                TzRect clip{0.0, 0.0, static_cast<double>(w), static_cast<double>(h)};
                TzPoint origin{0.0, 0.0};
                TzPainter painter(d->pixels.data(), w, h, clip, origin);
                TzPaintEvent pe(&painter);
                paintEvent(&pe);
                d->platformWindow->render(d->pixels, w, h);
            }
        }
    });
}

TzWindow::~TzWindow() {}

void TzWindow::setTitle(const std::string &title)
{
    TZ_D(TzWindow);
    d->platformWindow->setTitle(title);
}

void TzWindow::show()
{
    TZ_D(TzWindow);
    d->platformWindow->show();
    // Re-mark dirty so the paint timer fires a repaint on its first tick after
    // exec() starts.  platformWindow->show() fires a synchronous resize which
    // calls doPaint() and clears the dirty flag; marking it again ensures the
    // scene renders at least once inside the running event loop regardless of
    // whether the setNeedsDisplay: call inside show() is coalesced away.
    d->scene->markPaintDirty();
}

void TzWindow::hide()
{
    TZ_D(TzWindow);
    d->platformWindow->hide();
}

void TzWindow::close()
{
    TzCloseEvent e;
    closeEvent(&e);
}

TzScene *TzWindow::scene()
{
    TZ_D(TzWindow);
    return d->scene;
}

void TzWindow::setRootWidget(TzWidget *root)
{
    TZ_D(TzWindow);
    d->scene->setRoot(root);
    if (root)
        d->scene->setFocusedWidget(root);
}

TzWidget *TzWindow::rootWidget() const
{
    TZ_D(const TzWindow);
    return d->scene->root();
}

int TzWindow::width() const
{
    TZ_D(const TzWindow);
    return d->scene->width();
}

int TzWindow::height() const
{
    TZ_D(const TzWindow);
    return d->scene->height();
}

void TzWindow::update()
{
    TZ_D(TzWindow);
    d->paintDirty = true;
}

void TzWindow::setRhi(TzRhi *rhi, TzRhiSwapChain *swapChain)
{
    TZ_D(TzWindow);
    d->scene->setRhi(rhi, swapChain);
}

void TzWindow::paintEvent(TzPaintEvent * /*event*/) {}

void TzWindow::closeEvent(TzCloseEvent *event)
{
    TZ_D(TzWindow);
    d->scene->dispatchCloseEvent(event);
    hide();
}

void TzWindow::keyEvent(TzKeyEvent *event)
{
    TZ_D(TzWindow);
    d->scene->dispatchKeyEvent(event);
}

void TzWindow::mouseEvent(TzMouseEvent *event)
{
    TZ_D(TzWindow);
    d->scene->dispatchMouseEvent(event);
}

void TzWindow::resizeEvent(TzResizeEvent *event)
{
    TZ_D(TzWindow);
    d->scene->dispatchResizeEvent(event);
    // Only trigger the custom paint path on resize when the scene has no root
    // widget.  If a root widget is set, the scene already rendered to the
    // platform surface via doPaint(); setting paintDirty here would cause the
    // timer to call paintEvent() (often a no-op) and render() with an empty
    // pixel buffer, clobbering the scene's freshly-rendered pixels.
    if (!d->scene->root())
        d->paintDirty = true;
}

TzSurface::SurfaceType TzWindow::surfaceType() const
{
    return TzSurface::RasterSurface;
}

TzPlatformSurface *TzWindow::surfaceHandle()
{
    TZ_D(TzWindow);
    return d->platformWindow;
}

const TzPlatformSurface *TzWindow::surfaceHandle() const
{
    TZ_D(const TzWindow);
    return d->platformWindow;
}
