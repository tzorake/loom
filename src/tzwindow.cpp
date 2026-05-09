#include <event-loop/tzwindow.hpp>
#include <event-loop/tzsurface.hpp>
#include <event-loop/tzplatformsurface.hpp>
#include <event-loop/tzscene.hpp>
#include <event-loop/tzwidget.hpp>
#include <event-loop/tztimer.hpp>
#include <event-loop/tzcoreapplication.hpp>
#include <event-loop/tzabstractplatformintegration.hpp>
#include <event-loop/tzabstractwindow.hpp>

#include "tzwindow_p.hpp"

#include <chrono>

TzWindowPrivate::~TzWindowPrivate()
{
    delete paintTimer;
    // Clear scene callbacks before deleting the scene so the lambdas captured
    // inside TzScene don't dangle during platform window teardown.
    if (platformWindow) {
        platformWindow->setResizeCallback({});
        platformWindow->setKeyCallback({});
        platformWindow->setMouseCallback({});
        platformWindow->setCloseCallback({});
    }
    delete scene;
    delete platformWindow;
}

TzWindow::TzWindow(TzWindow *parent)
    : TzWindow(*new TzWindowPrivate, 400, 300, parent)
{
}

TzWindow::TzWindow(int width, int height, TzWindow *parent)
    : TzWindow(*new TzWindowPrivate, width, height, parent)
{
}

TzWindow::TzWindow(TzWindowPrivate &dd, int width, int height, TzWindow *parent)
    : TzObject(dd, parent)
{
    TZ_D(TzWindow);
    d->q_ptr = this;

    TzAbstractPlatformIntegration *platformIntegration = TzCoreApplication::instance()->platformIntegration();
    d->platformWindow = platformIntegration->createWindow(width, height);
    d->platformWindow->setSurface(this);

    d->scene = new TzScene(this);

    d->platformWindow->setCloseCallback([this] {
        TZ_D(TzWindow);
        closeEvent();
        if (d->onClose)
            d->onClose();
    });

    d->paintTimer = TzTimer::repeat(
        TzCoreApplication::instance()->eventDispatcher(),
        std::chrono::milliseconds(16),
        [this] {
            TZ_D(TzWindow);
            if (d->scene->isPaintDirty())
                d->scene->doPaint();
        });
}

TzWindow::~TzWindow()
{
}

void TzWindow::setTitle(const std::string &title)
{
    TZ_D(TzWindow);
    d->platformWindow->setTitle(title);
}

void TzWindow::show()
{
    TZ_D(TzWindow);
    d->platformWindow->show();
}

void TzWindow::hide()
{
    TZ_D(TzWindow);
    d->platformWindow->hide();
}

void TzWindow::close()
{
    TZ_D(TzWindow);
    closeEvent();
    if (d->onClose)
        d->onClose();
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

void TzWindow::setOnClose(std::function<void()> callback)
{
    TZ_D(TzWindow);
    d->onClose = std::move(callback);
}

void TzWindow::closeEvent()
{
    hide();
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
