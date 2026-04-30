#include <event-loop/tzeventloop.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>

#include "tzeventloop_p.hpp"

TzEventLoopPrivate::TzEventLoopPrivate(TzAbstractEventDispatcher *eventDispatcher)
    : eventDispatcher(eventDispatcher)
{
}

TzEventLoop::TzEventLoop(TzAbstractEventDispatcher *eventDispatcher)
    : d_ptr(new TzEventLoopPrivate(eventDispatcher))
{
    d_ptr->q_ptr = this;
}

TzEventLoop::~TzEventLoop()
{
}

void TzEventLoop::exec()
{
    d_ptr->eventDispatcher->processEvents();
}

void TzEventLoop::quit()
{
    d_ptr->eventDispatcher->interrupt();
}
