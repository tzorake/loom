#pragma once

#include "tzwebeventdispatcher.hpp"
#include <loom/tzclasshelpermacros.hpp>

#include <chrono>
#include <memory>
#include <unordered_map>

class TzWebEventDispatcherPrivate
{
    TZ_DECLARE_PUBLIC(TzWebEventDispatcher)
public:
    TzWebEventDispatcher *q_ptr{nullptr};

    TzAbstractEventDispatcher::PreWaitCallback preWaitCallback;

    // ── Timer table ───────────────────────────────────────────────────────
    struct TimerEntry
    {
        TzAbstractEventDispatcher::TimerCallback  callback;
        TzAbstractEventDispatcher::TimerInterval  interval;
        std::chrono::steady_clock::time_point     nextFire;
        bool                                      singleShot{false};
    };

    std::unordered_map<TzAbstractEventDispatcher::TimerHandle,
                       std::unique_ptr<TimerEntry>> timers;

    // ── Quit flag ─────────────────────────────────────────────────────────
    // Set by interrupt() when app.quit() is called.  tick() checks this and
    // stops calling js_request_animation_frame() so the RAF loop drains.
    bool quit{false};

    // ── Socket-notifier stub counter ──────────────────────────────────────
    // registerSocketNotifier() returns a unique non-null handle so callers
    // can safely call unregisterSocketNotifier() later without crashing.
    int nextNotifyId{1};
};
