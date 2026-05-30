#include "tzwebeventdispatcher.hpp"
#include "tzwebeventdispatcher_p.hpp"
#include "tzwebimports.hpp"

#include <vector>

// ── Module-level singleton ────────────────────────────────────────────────

static TzWebEventDispatcher *s_instance = nullptr;

// ── Exported entry point called from JavaScript ───────────────────────────
//
// The JavaScript loader calls this on every requestAnimationFrame callback:
//
//   requestAnimationFrame(() => instance.exports.loom_tick());

WASM_EXPORT_AS("loom_tick")
void loom_tick()
{
    if (s_instance)
        s_instance->tick();
}

// ── TzWebEventDispatcher ──────────────────────────────────────────────────

TzWebEventDispatcher::TzWebEventDispatcher()
    : d_ptr(new TzWebEventDispatcherPrivate)
{
    d_ptr->q_ptr = this;
    s_instance   = this;
}

TzWebEventDispatcher::~TzWebEventDispatcher()
{
    if (s_instance == this)
        s_instance = nullptr;
}

TzWebEventDispatcher *TzWebEventDispatcher::instance()
{
    return s_instance;
}

// processEvents() is called exactly once by TzEventLoop::exec().
// It performs the first tick and schedules the initial animation frame,
// effectively "starting" the loop before exec() returns to the caller.
void TzWebEventDispatcher::processEvents()
{
    tick();
}

void TzWebEventDispatcher::interrupt() {}  // RAF manages the loop's lifetime
void TzWebEventDispatcher::wakeUp()    {}  // no blocking wait to wake from

void TzWebEventDispatcher::setPreWaitCallback(PreWaitCallback callback)
{
    d_ptr->preWaitCallback = std::move(callback);
}

// ── Timer management ──────────────────────────────────────────────────────

TzWebEventDispatcher::TimerHandle
TzWebEventDispatcher::registerTimer(TimerInterval interval, bool singleShot,
                                    TimerCallback callback)
{
    auto entry        = std::make_unique<TzWebEventDispatcherPrivate::TimerEntry>();
    entry->callback   = std::move(callback);
    entry->interval   = interval;
    entry->singleShot = singleShot;
    entry->nextFire   = std::chrono::steady_clock::now() + interval;

    TimerHandle handle = static_cast<TimerHandle>(entry.get());
    d_ptr->timers[handle] = std::move(entry);
    return handle;
}

void TzWebEventDispatcher::unregisterTimer(TimerHandle handle)
{
    d_ptr->timers.erase(handle);
}

// ── Socket-notifier stub ──────────────────────────────────────────────────

TzWebEventDispatcher::NotifyHandle
TzWebEventDispatcher::registerSocketNotifier(int /*fd*/, NotifyCallback /*callback*/)
{
    // Return a unique non-null handle.  No actual I/O monitoring is set up:
    // the only caller on web is TzSignalHandler, which uses a no-op pipe.
    return reinterpret_cast<NotifyHandle>(
        static_cast<std::intptr_t>(d_ptr->nextNotifyId++));
}

void TzWebEventDispatcher::unregisterSocketNotifier(NotifyHandle /*handle*/) {}

// ── tick() ────────────────────────────────────────────────────────────────
//
// One animation frame:
//   1. Flush the posted-event queue (TzCoreApplication::processPostedEvents).
//   2. Fire all timers whose deadline has passed.
//   3. Schedule the next animation frame.

void TzWebEventDispatcher::tick()
{
    // 1. Process posted events (resize, paint, key, mouse, …).
    if (d_ptr->preWaitCallback)
        d_ptr->preWaitCallback();

    // 2. Fire due timers.
    //    Snapshot the keys first so callbacks can safely add/remove timers.
    auto now = std::chrono::steady_clock::now();

    std::vector<TimerHandle> toFire;
    toFire.reserve(d_ptr->timers.size());
    for (auto &[handle, entry] : d_ptr->timers) {
        if (now >= entry->nextFire)
            toFire.push_back(handle);
    }

    std::vector<TimerHandle> toRemove;
    for (auto handle : toFire) {
        auto it = d_ptr->timers.find(handle);
        if (it == d_ptr->timers.end())
            continue; // already removed by a previous callback

        it->second->callback();

        // Re-lookup: the callback may have unregistered this timer.
        auto it2 = d_ptr->timers.find(handle);
        if (it2 == d_ptr->timers.end())
            continue;

        if (it2->second->singleShot)
            toRemove.push_back(handle);
        else
            it2->second->nextFire = now + it2->second->interval;
    }

    for (auto handle : toRemove)
        d_ptr->timers.erase(handle);

    // 3. Ask the browser for the next frame.
    js_request_animation_frame();
}
