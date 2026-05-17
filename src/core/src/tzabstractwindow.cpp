#include <loom/tzabstractwindow.hpp>
#include <loom/tzcloseevent.hpp>
#include <loom/tzevent.hpp>
#include <loom/tzkeyevent.hpp>
#include <loom/tzmouseevent.hpp>
#include <loom/tzresizeevent.hpp>

#include "tzabstractwindow_p.hpp"

TzAbstractWindow::TzAbstractWindow(TzObject *parent)
    : TzObject(parent)
    , d_ptr(new TzAbstractWindowPrivate)
{
    d_ptr->q_ptr = this;
}

TzAbstractWindow::~TzAbstractWindow()
{
    delete d_ptr;
}

// ── TzPlatformSurface ─────────────────────────────────────────────────────

TzSurface *TzAbstractWindow::surface() const
{
    return d_ptr->surface;
}
void TzAbstractWindow::setSurface(TzSurface *s)
{
    d_ptr->surface = s;
}

void TzAbstractWindow::setCloseCallback(CloseCallback cb)
{
    d_ptr->closeCallback = std::move(cb);
}
void TzAbstractWindow::setResizeCallback(ResizeCallback cb)
{
    d_ptr->resizeCallback = std::move(cb);
}
void TzAbstractWindow::setKeyCallback(KeyCallback cb)
{
    d_ptr->keyCallback = std::move(cb);
}
void TzAbstractWindow::setMouseCallback(MouseCallback cb)
{
    d_ptr->mouseCallback = std::move(cb);
}

// ── TzObject::event ───────────────────────────────────────────────────────

bool TzAbstractWindow::event(TzEvent *e)
{
    switch (e->type()) {
    case TzEvent::WindowClose:
        if (d_ptr->closeCallback)
            d_ptr->closeCallback();
        return true;

    case TzEvent::WindowResize: {
        auto *re = static_cast<TzResizeEvent *>(e);
        if (d_ptr->resizeCallback)
            d_ptr->resizeCallback(re->width(), re->height());
        return true;
    }

    case TzEvent::KeyPress:
    case TzEvent::KeyRelease:
        if (d_ptr->keyCallback) {
            d_ptr->keyCallback(static_cast<TzKeyEvent *>(e));
            return true;
        }
        return false;

    case TzEvent::MouseMove:
    case TzEvent::MouseButtonPress:
    case TzEvent::MouseButtonRelease:
    case TzEvent::MouseScroll:
        if (d_ptr->mouseCallback) {
            d_ptr->mouseCallback(static_cast<TzMouseEvent *>(e));
            return true;
        }
        return false;

    default:
        return TzObject::event(e);
    }
}
