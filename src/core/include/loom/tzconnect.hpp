#ifndef TZCONNECT_HPP
#define TZCONNECT_HPP

#include <loom/tzeventemitter.hpp>
#include <loom/tzeventlistener.hpp>
#include <memory>
#include <type_traits>

#define TZ_DECLARE_SIGNAL(qualified_name)                                     \
    static constexpr const char *tzSignalName(                                \
        decltype(&qualified_name)) noexcept                                   \
    {                                                                         \
        return #qualified_name;                                               \
    }
    /**/

#define TZ_EMIT(qualified_name, ...)                                          \
    TzEventEmitter::emit(#qualified_name __VA_OPT__(, ) __VA_ARGS__)          \
    /**/

// Non-const member function slot
template <typename Sender, typename... SigArgs,
          typename Receiver, typename... SlotArgs>
TzEventListener connect(Sender *sender,
                        void (Sender::*signal)(SigArgs...),
                        Receiver *receiver,
                        void (Receiver::*slot)(SlotArgs...))
{
    static_assert(std::is_base_of_v<TzEventEmitter, Sender>,
                  "Sender must inherit from TzEventEmitter");
    static_assert(sizeof...(SlotArgs) <= sizeof...(SigArgs),
                  "Slot takes more arguments than the signal provides");
    const char *event = Sender::tzSignalName(signal);
    return static_cast<TzEventEmitter *>(sender)->on(
        event, [receiver, slot](SlotArgs... args) { (receiver->*slot)(args...); });
}

// Const member function slot
template <typename Sender, typename... SigArgs,
          typename Receiver, typename... SlotArgs>
TzEventListener connect(Sender *sender,
                        void (Sender::*signal)(SigArgs...),
                        Receiver *receiver,
                        void (Receiver::*slot)(SlotArgs...) const)
{
    static_assert(std::is_base_of_v<TzEventEmitter, Sender>,
                  "Sender must inherit from TzEventEmitter");
    static_assert(sizeof...(SlotArgs) <= sizeof...(SigArgs),
                  "Slot takes more arguments than the signal provides");
    const char *event = Sender::tzSignalName(signal);
    return static_cast<TzEventEmitter *>(sender)->on(
        event, [receiver, slot](SlotArgs... args) { (receiver->*slot)(args...); });
}

// Callable (lambda, free function pointer, std::function)
template <typename Sender, typename... SigArgs, typename Callable>
TzEventListener connect(Sender *sender,
                        void (Sender::*signal)(SigArgs...),
                        Callable &&cb)
{
    static_assert(std::is_base_of_v<TzEventEmitter, Sender>,
                  "Sender must inherit from TzEventEmitter");
    const char *event = Sender::tzSignalName(signal);
    return static_cast<TzEventEmitter *>(sender)->on(event, std::forward<Callable>(cb));
}

// shared_ptr<Receiver> — slot silently skips once the receiver is destroyed
template <typename Sender, typename... SigArgs,
          typename Receiver, typename... SlotArgs>
TzEventListener connect(Sender *sender,
                        void (Sender::*signal)(SigArgs...),
                        std::shared_ptr<Receiver> receiver,
                        void (Receiver::*slot)(SlotArgs...))
{
    static_assert(std::is_base_of_v<TzEventEmitter, Sender>,
                  "Sender must inherit from TzEventEmitter");
    static_assert(sizeof...(SlotArgs) <= sizeof...(SigArgs),
                  "Slot takes more arguments than the signal provides");
    const char *event = Sender::tzSignalName(signal);
    std::weak_ptr<Receiver> weak(receiver);
    return static_cast<TzEventEmitter *>(sender)->on(
        event, [weak = std::move(weak), slot](SlotArgs... args) {
            if (auto sp = weak.lock())
                (sp.get()->*slot)(args...);
        });
}

#endif // TZCONNECT_HPP
