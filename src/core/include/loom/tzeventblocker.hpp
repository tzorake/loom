#pragma once
#include <loom/tzeventemitter.hpp>

class TzEventBlocker final
{
public:
    explicit TzEventBlocker(TzEventEmitter &emitter)
        : m_emitter(emitter)
    {
        m_emitter.blockEvents(true);
    }

    ~TzEventBlocker()
    {
        m_emitter.blockEvents(false);
    }

    TzEventBlocker(const TzEventBlocker &) = delete;
    TzEventBlocker &operator=(const TzEventBlocker &) = delete;

private:
    TzEventEmitter &m_emitter;
};
