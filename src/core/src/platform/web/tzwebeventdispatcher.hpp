#pragma once

#include <loom/tzabstracteventdispatcher.hpp>
#include <loom/tzclasshelpermacros.hpp>

#include <memory>

class TzWebEventDispatcherPrivate;

// Web platform event dispatcher.
//
// Uses JSPI (JavaScript Promise Integration) to implement a blocking event
// loop inside a WebAssembly module.  js_yield() is a suspending import: each
// call suspends WASM until the browser's requestAnimationFrame callback
// resolves the pending Promise, then execution resumes.
//
// Design
// ──────
// • processEvents() — called once by TzEventLoop::exec().  Loops: yield to JS,
//   flush posted events, fire due timers, repeat until interrupt() is called.
//   Returns normally when quit, so loom_main() cleanup and stack destructors
//   fire without any special rewind machinery.
//
// • tick() — per-frame work: flushes the posted-event queue and fires timers.
//   Called from processEvents() after each js_yield() resumes.
//
// • interrupt() — sets the quit flag; processEvents() exits its loop on the
//   next iteration.
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
