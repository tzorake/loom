#ifndef TZWINDOWSEVENTDISPATCHER_P_HPP
#define TZWINDOWSEVENTDISPATCHER_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include "tzwindowseventdispatcher.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>

class TzWindowsEventDispatcherPrivate
{
    TZ_DECLARE_PUBLIC(TzWindowsEventDispatcher)
public:
    TzWindowsEventDispatcherPrivate();
    ~TzWindowsEventDispatcherPrivate();

    TzWindowsEventDispatcher *q_ptr{ nullptr };

    HANDLE wakeUpEvent{ nullptr };          // manual-reset event for interrupt/wakeUp
    std::atomic<bool> interrupted{ false };

    TzAbstractEventDispatcher::PreWaitCallback preWaitCallback;

    // Timers use WaitableTimer objects so they fit directly in MsgWaitForMultipleObjectsEx.
    struct TimerWrapper {
        HANDLE timerHandle{ nullptr };
        TzAbstractEventDispatcher::TimerCallback callback;
        bool     singleShot{ false };
        LONGLONG interval100ns{ 0 };        // for repeating: re-arm period in 100-ns units
        TzWindowsEventDispatcher *eventDispatcher{ nullptr };
    };
    using TimerWrapperPtr = std::unique_ptr<TimerWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::TimerHandle, TimerWrapperPtr> timerMap;

    // Socket notifiers — console handles need a monitor thread + regular event.
    struct NotifyWrapper {
        // What MsgWaitForMultipleObjectsEx waits on:
        //   regular handles  → raw OS handle
        //   console handles  → auto-reset event set by monitorThread
        HANDLE handle{ nullptr };
        HANDLE rawHandle{ nullptr };
        int    fd{ -1 };
        TzAbstractEventDispatcher::NotifyCallback callback;

        bool              isConsole{ false };
        HANDLE            notifyEvent{ nullptr };
        std::atomic<bool> threadRunning{ false };
        std::thread       monitorThread;
    };
    using NotifyWrapperPtr = std::unique_ptr<NotifyWrapper>;
    std::unordered_map<TzAbstractEventDispatcher::NotifyHandle, NotifyWrapperPtr> notifyMap;
};

#endif // TZWINDOWSEVENTDISPATCHER_P_HPP
