#ifndef TZX11EVENTDISPATCHER_P_HPP
#define TZX11EVENTDISPATCHER_P_HPP

#include "tzx11eventdispatcher.hpp"
#include <loom/tzclasshelpermacros.hpp>

#include <atomic>
#include <memory>
#include <unordered_map>

class TzX11EventDispatcherPrivate
{
    TZ_DECLARE_PUBLIC(TzX11EventDispatcher)
public:
    TzX11EventDispatcherPrivate();
    ~TzX11EventDispatcherPrivate();

    TzX11EventDispatcher *q_ptr{nullptr};

    int epollFd{-1};
    int wakeFd{-1};
    int x11Fd{-1}; // -1 until setX11Fd() is called

    std::atomic<bool> interrupted{false};

    TzAbstractEventDispatcher::PreWaitCallback preWaitCallback;

    struct TimerWrapper
    {
        int timerFd{-1};
        bool singleShot{false};
        TzAbstractEventDispatcher::TimerCallback callback;
    };
    using TimerWrapperPtr = std::unique_ptr<TimerWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::TimerHandle, TimerWrapperPtr> timerMap;

    struct NotifyWrapper
    {
        int fd{-1};
        TzAbstractEventDispatcher::NotifyCallback callback;
    };
    using NotifyWrapperPtr = std::unique_ptr<NotifyWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::NotifyHandle, NotifyWrapperPtr> notifyMap;
};

#endif // TZX11EVENTDISPATCHER_P_HPP
