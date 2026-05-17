#pragma once
#include <loom/tzeventlistener.hpp>

class TzScopedEventListener final
{
public:
    TzScopedEventListener();
    TzScopedEventListener(TzEventListener listener);

    TzScopedEventListener(TzScopedEventListener&& other) noexcept;
    TzScopedEventListener& operator=(TzScopedEventListener&& other) noexcept;
    TzScopedEventListener(const TzScopedEventListener&) = delete;
    TzScopedEventListener& operator=(const TzScopedEventListener&) = delete;

    ~TzScopedEventListener();

    operator bool() const;

    void swap(TzScopedEventListener& other) noexcept;
    TzEventListener release() noexcept;

private:
    TzEventListener m_listener;
};
