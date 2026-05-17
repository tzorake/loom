#pragma once
#include <loom/tzeventemitter.hpp>
#include <list>
#include <unordered_map>

class TzEventEmitterPrivate : public std::enable_shared_from_this<TzEventEmitterPrivate>
{
public:
    using ListenerCallback = TzEventEmitter::ListenerCallback;

    TzEventEmitterPrivate();

    void setMaxListeners(std::size_t n);
    std::size_t getMaxListeners() const;

    std::pair<std::uint64_t, std::string> addListener(const std::string &eventName,
                                                      ListenerCallback callback);
    std::pair<std::uint64_t, std::string> addOnceListener(const std::string &eventName,
                                                          ListenerCallback callback);

    bool removeListener(const TzEventListener &listener);
    void removeAllListeners(const std::string &eventName);
    void emitPacked(const std::string &eventName, const std::vector<std::any> &args);

    std::vector<std::string> eventNames() const;
    std::size_t listenerCount(const std::string &eventName) const;

    void checkLimit(const std::string &event, std::size_t count);

    std::unordered_map<std::string, std::list<std::pair<std::uint64_t, ListenerCallback>>> listeners;
    std::uint64_t nextId{1};
    std::size_t maxListeners = defaultMaxListeners;
    static inline std::size_t defaultMaxListeners = 10;
};
