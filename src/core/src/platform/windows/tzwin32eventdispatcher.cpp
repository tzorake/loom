#include "tzwin32eventdispatcher.hpp"
#include "tzwin32eventdispatcher_p.hpp"

#include <io.h>
#include <stdexcept>
#include <vector>

TzWin32EventDispatcherPrivate::TzWin32EventDispatcherPrivate()
{
    // Manual-reset event: stays set until we explicitly reset it.
    wakeUpEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!wakeUpEvent)
        throw std::runtime_error("CreateEvent (wakeUpEvent) failed");
}

TzWin32EventDispatcherPrivate::~TzWin32EventDispatcherPrivate()
{
    if (wakeUpEvent) {
        CloseHandle(wakeUpEvent);
        wakeUpEvent = nullptr;
    }
}

TzWin32EventDispatcher::TzWin32EventDispatcher()
    : d_ptr(new TzWin32EventDispatcherPrivate)
{
    d_ptr->q_ptr = this;
}

TzWin32EventDispatcher::~TzWin32EventDispatcher()
{
    while (!d_ptr->timerMap.empty())
        unregisterTimer(d_ptr->timerMap.begin()->first);
    while (!d_ptr->notifyMap.empty())
        unregisterSocketNotifier(d_ptr->notifyMap.begin()->first);
}

void TzWin32EventDispatcher::processEvents()
{
    d_ptr->interrupted = false;

    while (!d_ptr->interrupted) {
        if (d_ptr->preWaitCallback)
            d_ptr->preWaitCallback();

        // Build handles: wakeUp + one per timer + one per notifier.
        std::vector<HANDLE> handles;
        handles.reserve(1 + d_ptr->timerMap.size() + d_ptr->notifyMap.size());
        handles.push_back(d_ptr->wakeUpEvent);
        for (auto &[_, w] : d_ptr->timerMap)
            handles.push_back(w->timerHandle);
        for (auto &[_, w] : d_ptr->notifyMap)
            handles.push_back(w->handle);

        DWORD count = static_cast<DWORD>(handles.size());
        DWORD result = MsgWaitForMultipleObjectsEx(count, handles.data(), INFINITE, QS_ALLINPUT,
                                                   MWMO_ALERTABLE);

        if (result == WAIT_OBJECT_0) {
            // wakeUpEvent — reset and re-check interrupted flag at the loop top.
            ResetEvent(d_ptr->wakeUpEvent);

        } else if (result > WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + count) {
            HANDLE signaled = handles[result - WAIT_OBJECT_0];

            // Timer? Extract callback before calling (callback may modify timerMap).
            {
                TimerHandle th{};
                TzAbstractEventDispatcher::TimerCallback cb;
                bool single = false;
                for (auto &[h, w] : d_ptr->timerMap) {
                    if (w->timerHandle == signaled) {
                        th = h;
                        cb = w->callback;
                        single = w->singleShot;
                        break;
                    }
                }
                if (cb) {
                    cb();
                    if (single)
                        unregisterTimer(th);
                }
            }

            // Notify? Same pattern.
            {
                TzAbstractEventDispatcher::NotifyCallback cb;
                int fd = -1;
                for (auto &[_, w] : d_ptr->notifyMap) {
                    if (w->handle == signaled) {
                        cb = w->callback;
                        fd = w->fd;
                        break;
                    }
                }
                if (cb)
                    cb(fd);
            }

        } else if (result == WAIT_OBJECT_0 + count) {
            // Message queue — dispatch window messages (WM_PAINT, WM_KEY*, etc.).
            MSG msg;
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    d_ptr->interrupted = true;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
        // WAIT_IO_COMPLETION, WAIT_FAILED → fall through to next iteration.
    }
}

void TzWin32EventDispatcher::interrupt()
{
    d_ptr->interrupted = true;
    SetEvent(d_ptr->wakeUpEvent);
}

void TzWin32EventDispatcher::wakeUp()
{
    SetEvent(d_ptr->wakeUpEvent);
}

void TzWin32EventDispatcher::setPreWaitCallback(PreWaitCallback callback)
{
    d_ptr->preWaitCallback = std::move(callback);
}

TzWin32EventDispatcher::TimerHandle TzWin32EventDispatcher::registerTimer(TimerInterval interval,
                                                                          bool singleShot,
                                                                          TimerCallback callback)
{
    // Synchronisation (auto-reset) waitable timer: resets automatically when
    // a waiting thread is released, so it won't keep MsgWaitForMultipleObjectsEx
    // spinning on every iteration for repeating timers.
    HANDLE hTimer = CreateWaitableTimerW(nullptr, FALSE, nullptr);
    if (!hTimer)
        throw std::runtime_error("CreateWaitableTimerW failed");

    // Negative = relative time, in 100-nanosecond units.
    LONGLONG interval100ns = (LONGLONG) interval.count() * 10000LL;
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -interval100ns;

    // period > 0 makes the OS re-arm automatically each period (ms).
    LONG periodMs = singleShot ? 0 : static_cast<LONG>(interval.count());
    if (!SetWaitableTimer(hTimer, &dueTime, periodMs, nullptr, nullptr, FALSE)) {
        CloseHandle(hTimer);
        throw std::runtime_error("SetWaitableTimer failed");
    }

    auto wrapper = std::make_unique<TzWin32EventDispatcherPrivate::TimerWrapper>();
    wrapper->timerHandle = hTimer;
    wrapper->callback = std::move(callback);
    wrapper->singleShot = singleShot;
    wrapper->interval100ns = interval100ns;
    wrapper->eventDispatcher = this;

    TimerHandle handle = static_cast<TimerHandle>(wrapper.get());
    d_ptr->timerMap[handle] = std::move(wrapper);
    return handle;
}

void TzWin32EventDispatcher::unregisterTimer(TimerHandle handle)
{
    auto it = d_ptr->timerMap.find(handle);
    if (it == d_ptr->timerMap.end())
        return;

    CancelWaitableTimer(it->second->timerHandle);
    CloseHandle(it->second->timerHandle);
    d_ptr->timerMap.erase(it);
}

TzWin32EventDispatcher::NotifyHandle TzWin32EventDispatcher::registerSocketNotifier(
    int fd, NotifyCallback callback)
{
    HANDLE osHandle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
    if (osHandle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("_get_osfhandle failed");

    auto wrapper = std::make_unique<TzWin32EventDispatcherPrivate::NotifyWrapper>();
    wrapper->rawHandle = osHandle;
    wrapper->fd = fd;
    wrapper->callback = std::move(callback);

    // Console input and anonymous pipe handles cannot be used with
    // MsgWaitForMultipleObjectsEx (console handles require ReadConsoleInput;
    // anonymous pipe handles are always in the signaled state).
    // Route both through a monitor thread + auto-reset event.
    DWORD consoleMode;
    bool isConsole = GetConsoleMode(osHandle, &consoleMode);
    DWORD fileType = GetFileType(osHandle);
    bool isPipe = (fileType == FILE_TYPE_PIPE);

    if (isConsole || isPipe) {
        wrapper->isConsole = true;
        wrapper->notifyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr); // auto-reset
        wrapper->handle = wrapper->notifyEvent;
        wrapper->threadRunning.store(true, std::memory_order_relaxed);

        HANDLE rawH = osHandle;
        HANDLE notEv = wrapper->notifyEvent;
        auto *flag = &wrapper->threadRunning;

        wrapper->monitorThread = std::thread([rawH, notEv, flag, isPipe]() {
            while (flag->load(std::memory_order_relaxed)) {
                if (isPipe) {
                    // Anonymous pipe handles are always signaled; use PeekNamedPipe
                    // to check for available data without consuming it.
                    DWORD available = 0;
                    if (PeekNamedPipe(rawH, nullptr, 0, nullptr, &available, nullptr)
                        && available > 0) {
                        SetEvent(notEv);
                    }
                    Sleep(10);
                } else {
                    // Console: wait up to 200 ms so we notice the stop flag promptly.
                    DWORD r = WaitForSingleObject(rawH, 200);
                    if (r == WAIT_OBJECT_0 && flag->load(std::memory_order_relaxed))
                        SetEvent(notEv);
                }
            }
        });
    } else {
        wrapper->handle = osHandle;
    }

    NotifyHandle handle = static_cast<NotifyHandle>(wrapper.get());
    d_ptr->notifyMap[handle] = std::move(wrapper);
    return handle;
}

void TzWin32EventDispatcher::unregisterSocketNotifier(NotifyHandle handle)
{
    auto it = d_ptr->notifyMap.find(handle);
    if (it == d_ptr->notifyMap.end())
        return;

    auto &w = *it->second;
    if (w.isConsole) {
        w.threadRunning.store(false, std::memory_order_relaxed);
        if (w.monitorThread.joinable())
            w.monitorThread.join();
        CloseHandle(w.notifyEvent);
    }
    d_ptr->notifyMap.erase(it);
}
