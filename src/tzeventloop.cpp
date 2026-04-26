#include <event-loop/tzeventloop.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>

#include "tzeventloop_p.hpp"

TzEventLoopPrivate::TzEventLoopPrivate(TzAbstractEventDispatcher *dispatcher)
    : dispatcher(dispatcher)
{
}

TzEventLoop::TzEventLoop(TzAbstractEventDispatcher *dispatcher)
    : d_ptr(new TzEventLoopPrivate(dispatcher))
{
    d_ptr->q_ptr = this;
}

TzEventLoop::~TzEventLoop()
{
}

void TzEventLoop::exec()
{
    d_ptr->dispatcher->processEvents();
}

void TzEventLoop::quit()
{
    d_ptr->dispatcher->interrupt();
}
