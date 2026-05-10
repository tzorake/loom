#include <loom/tzsocketnotifier.hpp>
#include <loom/tzabstracteventdispatcher.hpp>

#include "tzsocketnotifier_p.hpp"

TzSocketNotifierPrivate::TzSocketNotifierPrivate(TzAbstractEventDispatcher *eventDispatcher)
    : eventDispatcher(eventDispatcher)
{
}

TzSocketNotifier::TzSocketNotifier(TzAbstractEventDispatcher *eventDispatcher)
    : d_ptr(new TzSocketNotifierPrivate(eventDispatcher))
{
}

TzSocketNotifier::~TzSocketNotifier()
{
    stop();
}

TzSocketNotifier::NotifyHandle TzSocketNotifier::handle() const
{
    return d_ptr->handle;
}

void TzSocketNotifier::setFd(int fd)
{
    d_ptr->fd = fd;
}

int TzSocketNotifier::fd() const
{
    return d_ptr->fd;
}

void TzSocketNotifier::setCallback(NotifyCallback callback)
{
    d_ptr->callback = std::move(callback);
}

void TzSocketNotifier::start()
{
    d_ptr->handle = d_ptr->eventDispatcher->registerSocketNotifier(d_ptr->fd, d_ptr->callback);
}

void TzSocketNotifier::stop()
{
    d_ptr->eventDispatcher->unregisterSocketNotifier(d_ptr->handle);
}
