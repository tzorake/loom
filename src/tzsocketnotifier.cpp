#include <event-loop/tzsocketnotifier.hpp>
#include <event-loop/tzabstracteventdispatcher.hpp>

#include "tzsocketnotifier_p.hpp"

TzSocketNotifierPrivate::TzSocketNotifierPrivate(TzAbstractEventDispatcher *dispatcher)
    : dispatcher(dispatcher)
{
}

TzSocketNotifier::TzSocketNotifier(TzAbstractEventDispatcher *dispatcher)
    : d_ptr(new TzSocketNotifierPrivate(dispatcher))
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
    d_ptr->handle = d_ptr->dispatcher->registerSocketNotifier(d_ptr->fd, d_ptr->callback);
}

void TzSocketNotifier::stop()
{
    d_ptr->dispatcher->unregisterSocketNotifier(d_ptr->handle);
}
