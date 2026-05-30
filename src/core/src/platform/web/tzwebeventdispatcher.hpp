#pragma once

#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzclasshelpermacros.hpp>

#include <memory>

class TzWebEventDispatcherPrivate;

// Web platform event dispatcher.
//
// The browser's event loop is driven by requestAnimationFrame (RAF); there is
// no way to block a WASM thread waiting for OS events the way a POSIX epoll or
// Windows MsgWaitForMultipleObjects call would.
//
// Design
// ──────
// • processEvents() — called once by TzEventLoop::exec() to start the loop.
//   It performs one tick (flush posted events, fire timers) and schedules the
//   next animation frame via js_request_animation_frame().
//
// • tick() — the real per-frame work-horse.  The exported loom_tick() symbol
//   (defined in the .cpp) calls this every RAF.
//
// • interrupt() / wakeUp() — no-ops; the RAF cycle manages lifetime.
//
// • registerSocketNotifier() — stub that returns a dummy handle.
//   (The only use on web is TzSignalHandler, which routes through an in-memory
//   no-op TzSignalPipe; actual socket I/O is never needed in the browser.)

class TzWebEventDispatcher : public TzAbstractEventDispatcher
{
    TZ_DECLARE_PRIVATE(TzWebEventDispatcher)
public:
    TzWebEventDispatcher();
    ~TzWebEventDispatcher() override;

    // TzAbstractEventDispatcher interface
    void processEvents()                              override;
    void interrupt()                                  override;
    void wakeUp()                                     override;
    void setPreWaitCallback(PreWaitCallback callback) override;

    TimerHandle  registerTimer(TimerInterval interval, bool singleShot,
                               TimerCallback callback)        override;
    void         unregisterTimer(TimerHandle handle)          override;

    NotifyHandle registerSocketNotifier(int fd,
                                        NotifyCallback callback) override;
    void         unregisterSocketNotifier(NotifyHandle handle)   override;

    // Called from the exported loom_tick() on every animation frame.
    void tick();

    // Module-level singleton accessor.
    static TzWebEventDispatcher *instance();

private:
    std::unique_ptr<TzWebEventDispatcherPrivate> d_ptr;
};
