#include <loom/tzsocketnotifier.hpp>
#include <tzsocketnotifier_p.hpp>
#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzcoreapplication.hpp>

TzSocketNotifierPrivate::TzSocketNotifierPrivate()
{}

TzSocketNotifier::TzSocketNotifier()
    : d_ptr(new TzSocketNotifierPrivate)
{}

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
    TzAbstractEventDispatcher *eventDispatcher = tzApp->eventDispatcher();
    d_ptr->handle = eventDispatcher->registerSocketNotifier(d_ptr->fd, d_ptr->callback);
}

void TzSocketNotifier::stop()
{
    TzAbstractEventDispatcher *eventDispatcher = tzApp->eventDispatcher();
    eventDispatcher->unregisterSocketNotifier(d_ptr->handle);
}
