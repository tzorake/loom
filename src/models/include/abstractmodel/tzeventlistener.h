#pragma once
#include <memory>
#include <string>

class TzEventEmitter;
class TzEventEmitterPrivate;

class TzEventListener final
{
public:
    TzEventListener();
    ~TzEventListener();

    void disconnect();

    bool isConnected() const;
    operator bool() const;

    void swap(TzEventListener& other) noexcept;

private:
    friend TzEventEmitter;
    friend TzEventEmitterPrivate;
    TzEventListener(std::weak_ptr<TzEventEmitterPrivate> impl, std::string event, std::uint64_t id);

    std::weak_ptr<TzEventEmitterPrivate> m_impl;
    std::string m_event;
    std::uint64_t m_id = 0;
};
