#include "tzx11eventdispatcher.hpp"
#include "tzx11eventdispatcher_p.hpp"
#include "tzx11globals.hpp"

#include <errno.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

TzX11EventDispatcherPrivate::TzX11EventDispatcherPrivate()
{
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd < 0)
        throw std::runtime_error("epoll_create1 failed");

    wakeFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (wakeFd < 0)
        throw std::runtime_error("eventfd failed");

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = wakeFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, wakeFd, &ev);
}

TzX11EventDispatcherPrivate::~TzX11EventDispatcherPrivate()
{
    if (wakeFd >= 0)
        close(wakeFd);
    if (epollFd >= 0)
        close(epollFd);
}

// ── TzX11EventDispatcher ──────────────────────────────────────────────────────

TzX11EventDispatcher::TzX11EventDispatcher()
    : d_ptr(new TzX11EventDispatcherPrivate)
{
    d_ptr->q_ptr = this;
}

TzX11EventDispatcher::~TzX11EventDispatcher()
{
    while (!d_ptr->timerMap.empty())
        unregisterTimer(d_ptr->timerMap.begin()->first);
    while (!d_ptr->notifyMap.empty())
        unregisterSocketNotifier(d_ptr->notifyMap.begin()->first);
}

void TzX11EventDispatcher::setX11Fd(int fd)
{
    if (fd < 0 || d_ptr->x11Fd >= 0)
        return;

    d_ptr->x11Fd = fd;

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_ADD, fd, &ev);
}

void TzX11EventDispatcher::processEvents()
{
    d_ptr->interrupted = false;

    while (!d_ptr->interrupted) {
        // Drain any X11 events buffered by Xlib before blocking.
        TzX11Globals::instance().dispatchPendingEvents();

        if (d_ptr->interrupted)
            break;

        if (d_ptr->preWaitCallback)
            d_ptr->preWaitCallback();

        if (d_ptr->interrupted)
            break;

        epoll_event events[64];
        int n = epoll_wait(d_ptr->epollFd, events, 64, -1);

        if (n < 0) {
            if (errno == EINTR)
                continue;
            break;
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (fd == d_ptr->x11Fd) {
                TzX11Globals::instance().dispatchPendingEvents();

            } else if (fd == d_ptr->wakeFd) {
                uint64_t val;
                (void) read(d_ptr->wakeFd, &val, sizeof(val));

            } else {
                // Timer?
                {
                    TzAbstractEventDispatcher::TimerHandle th{};
                    TzAbstractEventDispatcher::TimerCallback cb;
                    bool single = false;
                    for (auto &[h, w] : d_ptr->timerMap) {
                        if (w->timerFd == fd) {
                            th = h;
                            cb = w->callback;
                            single = w->singleShot;
                            break;
                        }
                    }
                    if (cb) {
                        uint64_t exp;
                        (void) read(fd, &exp, sizeof(exp));
                        cb();
                        if (single)
                            unregisterTimer(th);
                        continue;
                    }
                }

                // Socket notifier?
                {
                    TzAbstractEventDispatcher::NotifyCallback cb;
                    for (auto &[_, w] : d_ptr->notifyMap) {
                        if (w->fd == fd) {
                            cb = w->callback;
                            break;
                        }
                    }
                    if (cb)
                        cb(fd);
                }
            }
        }
    }
}

void TzX11EventDispatcher::interrupt()
{
    d_ptr->interrupted = true;
    uint64_t val = 1;
    (void) write(d_ptr->wakeFd, &val, sizeof(val));
}

void TzX11EventDispatcher::wakeUp()
{
    uint64_t val = 1;
    (void) write(d_ptr->wakeFd, &val, sizeof(val));
}

void TzX11EventDispatcher::setPreWaitCallback(PreWaitCallback callback)
{
    d_ptr->preWaitCallback = std::move(callback);
}

TzX11EventDispatcher::TimerHandle TzX11EventDispatcher::registerTimer(TimerInterval interval,
                                                                      bool singleShot,
                                                                      TimerCallback callback)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (tfd < 0)
        throw std::runtime_error("timerfd_create failed");

    long secs = interval.count() / 1000;
    long nsecs = (interval.count() % 1000) * 1000000L;

    itimerspec spec{};
    spec.it_value.tv_sec = secs;
    spec.it_value.tv_nsec = nsecs;
    if (!singleShot) {
        spec.it_interval.tv_sec = secs;
        spec.it_interval.tv_nsec = nsecs;
    }
    timerfd_settime(tfd, 0, &spec, nullptr);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_ADD, tfd, &ev);

    auto wrapper = std::make_unique<TzX11EventDispatcherPrivate::TimerWrapper>();
    wrapper->timerFd = tfd;
    wrapper->singleShot = singleShot;
    wrapper->callback = std::move(callback);

    TimerHandle handle = static_cast<TimerHandle>(wrapper.get());
    d_ptr->timerMap[handle] = std::move(wrapper);
    return handle;
}

void TzX11EventDispatcher::unregisterTimer(TimerHandle handle)
{
    auto it = d_ptr->timerMap.find(handle);
    if (it == d_ptr->timerMap.end())
        return;

    int tfd = it->second->timerFd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_DEL, tfd, nullptr);
    close(tfd);
    d_ptr->timerMap.erase(it);
}

TzX11EventDispatcher::NotifyHandle TzX11EventDispatcher::registerSocketNotifier(
    int fd, NotifyCallback callback)
{
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_ADD, fd, &ev);

    auto wrapper = std::make_unique<TzX11EventDispatcherPrivate::NotifyWrapper>();
    wrapper->fd = fd;
    wrapper->callback = std::move(callback);

    NotifyHandle handle = static_cast<NotifyHandle>(wrapper.get());
    d_ptr->notifyMap[handle] = std::move(wrapper);
    return handle;
}

void TzX11EventDispatcher::unregisterSocketNotifier(NotifyHandle handle)
{
    auto it = d_ptr->notifyMap.find(handle);
    if (it == d_ptr->notifyMap.end())
        return;

    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_DEL, it->second->fd, nullptr);
    d_ptr->notifyMap.erase(it);
}
