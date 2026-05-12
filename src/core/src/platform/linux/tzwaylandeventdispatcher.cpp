#include "tzwaylandeventdispatcher.hpp"
#include "tzwaylandeventdispatcher_p.hpp"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>

TzWaylandEventDispatcherPrivate::TzWaylandEventDispatcherPrivate()
{
    epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd < 0)
        throw std::runtime_error("epoll_create1 failed");

    wakeFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
    if (wakeFd < 0)
        throw std::runtime_error("eventfd failed");

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = wakeFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, wakeFd, &ev);
    // Wayland display fd is added later via setWaylandDisplay().
}

TzWaylandEventDispatcherPrivate::~TzWaylandEventDispatcherPrivate()
{
    if (wakeFd >= 0)  close(wakeFd);
    if (epollFd >= 0) close(epollFd);
}

// ── TzWaylandEventDispatcher ─────────────────────────────────────────────────

TzWaylandEventDispatcher::TzWaylandEventDispatcher()
    : d_ptr(new TzWaylandEventDispatcherPrivate)
{
    d_ptr->q_ptr = this;
}

TzWaylandEventDispatcher::~TzWaylandEventDispatcher()
{
    while (!d_ptr->timerMap.empty())
        unregisterTimer(d_ptr->timerMap.begin()->first);
    while (!d_ptr->notifyMap.empty())
        unregisterSocketNotifier(d_ptr->notifyMap.begin()->first);
}

void TzWaylandEventDispatcher::setWaylandDisplay(wl_display *display)
{
    if (!display || d_ptr->waylandDisplay)
        return;

    d_ptr->waylandDisplay = display;
    d_ptr->waylandFd = wl_display_get_fd(display);

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = d_ptr->waylandFd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_ADD, d_ptr->waylandFd, &ev);
}

void TzWaylandEventDispatcher::processEvents()
{
    wl_display *display  = d_ptr->waylandDisplay; // null for console-only apps
    int         wlFd     = d_ptr->waylandFd;

    d_ptr->interrupted = false;
    bool needsFlush = false; // true when wl_display_flush returned EAGAIN

    while (!d_ptr->interrupted) {
        // Drain already-queued Wayland events before sleeping.
        if (display)
            wl_display_dispatch_pending(display);

        if (d_ptr->interrupted)
            break;

        if (d_ptr->preWaitCallback)
            d_ptr->preWaitCallback();

        if (d_ptr->interrupted)
            break;

        if (display) {
            // Drain any pending events that arrived since we last dispatched.
            while (wl_display_prepare_read(display) != 0) {
                wl_display_dispatch_pending(display);
                if (d_ptr->interrupted) break;
            }
            if (d_ptr->interrupted) {
                wl_display_cancel_read(display);
                break;
            }

            // Flush pending requests to the compositor.  If the socket buffer is
            // full (EAGAIN), watch for EPOLLOUT so we can retry once it drains.
            if (wl_display_flush(display) < 0 && errno == EAGAIN) {
                if (!needsFlush) {
                    epoll_event mod{};
                    mod.events  = EPOLLIN | EPOLLOUT;
                    mod.data.fd = wlFd;
                    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_MOD, wlFd, &mod);
                    needsFlush = true;
                }
            }
        }

        epoll_event events[64];
        int n = epoll_wait(d_ptr->epollFd, events, 64, -1);

        if (n < 0) {
            if (display) wl_display_cancel_read(display);
            if (errno == EINTR) continue;
            break;
        }

        // Inspect the Wayland fd events before touching the read queue.
        bool waylandReadable  = false;
        bool waylandWritable  = false;
        if (display) {
            for (int i = 0; i < n; i++) {
                if (events[i].data.fd == wlFd) {
                    waylandReadable = (events[i].events & EPOLLIN)  != 0;
                    waylandWritable = (events[i].events & EPOLLOUT) != 0;
                    break;
                }
            }

            // Retry flush when the socket becomes writable again.
            if (waylandWritable && needsFlush) {
                wl_display_flush(display);
                epoll_event mod{};
                mod.events  = EPOLLIN;
                mod.data.fd = wlFd;
                epoll_ctl(d_ptr->epollFd, EPOLL_CTL_MOD, wlFd, &mod);
                needsFlush = false;
            }

            if (waylandReadable) {
                wl_display_read_events(display);
                // Dispatch immediately so buffer_release events reach the SHM
                // buffers before any timer callbacks that may call render().
                wl_display_dispatch_pending(display);
            } else {
                wl_display_cancel_read(display);
            }
        }

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

            if (display && fd == wlFd) {
                // Already dispatched above after read_events.

            } else if (fd == d_ptr->wakeFd) {
                uint64_t val;
                (void)read(d_ptr->wakeFd, &val, sizeof(val));

            } else {
                // Timer?
                {
                    TzAbstractEventDispatcher::TimerHandle   th{};
                    TzAbstractEventDispatcher::TimerCallback cb;
                    bool single = false;
                    for (auto &[h, w] : d_ptr->timerMap) {
                        if (w->timerFd == fd) {
                            th     = h;
                            cb     = w->callback;
                            single = w->singleShot;
                            break;
                        }
                    }
                    if (cb) {
                        uint64_t exp;
                        (void)read(fd, &exp, sizeof(exp));
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
                        if (w->fd == fd) { cb = w->callback; break; }
                    }
                    if (cb) cb(fd);
                }
            }
        }
    }
}

void TzWaylandEventDispatcher::interrupt()
{
    d_ptr->interrupted = true;
    uint64_t val = 1;
    (void)write(d_ptr->wakeFd, &val, sizeof(val));
}

void TzWaylandEventDispatcher::wakeUp()
{
    uint64_t val = 1;
    (void)write(d_ptr->wakeFd, &val, sizeof(val));
}

void TzWaylandEventDispatcher::setPreWaitCallback(PreWaitCallback callback)
{
    d_ptr->preWaitCallback = std::move(callback);
}

TzWaylandEventDispatcher::TimerHandle TzWaylandEventDispatcher::registerTimer(
    TimerInterval interval, bool singleShot, TimerCallback callback)
{
    int tfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (tfd < 0)
        throw std::runtime_error("timerfd_create failed");

    long secs  = interval.count() / 1000;
    long nsecs = (interval.count() % 1000) * 1000000L;

    itimerspec spec{};
    spec.it_value.tv_sec  = secs;
    spec.it_value.tv_nsec = nsecs;
    if (!singleShot) {
        spec.it_interval.tv_sec  = secs;
        spec.it_interval.tv_nsec = nsecs;
    }
    timerfd_settime(tfd, 0, &spec, nullptr);

    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_ADD, tfd, &ev);

    auto wrapper        = std::make_unique<TzWaylandEventDispatcherPrivate::TimerWrapper>();
    wrapper->timerFd    = tfd;
    wrapper->singleShot = singleShot;
    wrapper->callback   = std::move(callback);

    TimerHandle handle = static_cast<TimerHandle>(wrapper.get());
    d_ptr->timerMap[handle] = std::move(wrapper);
    return handle;
}

void TzWaylandEventDispatcher::unregisterTimer(TimerHandle handle)
{
    auto it = d_ptr->timerMap.find(handle);
    if (it == d_ptr->timerMap.end())
        return;

    int tfd = it->second->timerFd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_DEL, tfd, nullptr);
    close(tfd);
    d_ptr->timerMap.erase(it);
}

TzWaylandEventDispatcher::NotifyHandle TzWaylandEventDispatcher::registerSocketNotifier(
    int fd, NotifyCallback callback)
{
    epoll_event ev{};
    ev.events  = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_ADD, fd, &ev);

    auto wrapper      = std::make_unique<TzWaylandEventDispatcherPrivate::NotifyWrapper>();
    wrapper->fd       = fd;
    wrapper->callback = std::move(callback);

    NotifyHandle handle = static_cast<NotifyHandle>(wrapper.get());
    d_ptr->notifyMap[handle] = std::move(wrapper);
    return handle;
}

void TzWaylandEventDispatcher::unregisterSocketNotifier(NotifyHandle handle)
{
    auto it = d_ptr->notifyMap.find(handle);
    if (it == d_ptr->notifyMap.end())
        return;

    epoll_ctl(d_ptr->epollFd, EPOLL_CTL_DEL, it->second->fd, nullptr);
    d_ptr->notifyMap.erase(it);
}
