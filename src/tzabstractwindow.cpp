#include <event-loop/tzabstractwindow.hpp>
#include <event-loop/tzevent.hpp>
#include <event-loop/tzkeyevent.hpp>
#include <event-loop/tzmouseevent.hpp>

#include "tzabstractwindow_p.hpp"

TzAbstractWindowPrivate::TzAbstractWindowPrivate()
{
}

TzAbstractWindow::TzAbstractWindow(TzObject *parent)
    : TzObject(parent)
    , d_ptr(new TzAbstractWindowPrivate)
{
}

TzAbstractWindow::~TzAbstractWindow()
{
}

void TzAbstractWindow::setKeyCallback(KeyCallback callback)
{
    d_ptr->keyCallback = std::move(callback);
}

void TzAbstractWindow::setMouseCallback(MouseCallback callback)
{
    d_ptr->mouseCallback = std::move(callback);
}

bool TzAbstractWindow::event(TzEvent *e)
{
    switch (e->type()) {
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
