#pragma once
#include <models/tzeventlistener.h>
#include <memory>
#include <any>
#include <functional>
#include <string>
#include <type_traits>
#include <tuple>
#include <vector>

class TzEventEmitterPrivate;

class TzEventEmitter
{
public:
    using ListenerCallback = std::function<void(const std::vector<std::any>&)>;

    TzEventEmitter();
    ~TzEventEmitter();

    template <typename Callable>
    TzEventListener on(const std::string& eventName, Callable&& callback);
    
    template <typename Callable>
    TzEventListener once(const std::string& eventName, Callable&& callback);

    bool removeListener(const TzEventListener& listener);
    void removeAllListeners(const std::string& eventName = {});

    template <typename... Args>
    void emit(const std::string& eventName, Args &&... args);

    std::vector<std::string> eventNames() const;
    std::size_t listenerCount(const std::string& eventName) const;

    void setMaxListeners(std::size_t n);
    std::size_t getMaxListeners() const;

    static void setDefaultMaxListeners(std::size_t n);
    static std::size_t getDefaultMaxListeners();

private:
    TzEventListener on(const std::string& eventName, ListenerCallback callback);
    TzEventListener once(const std::string& eventName, ListenerCallback callback);
    void emit(const std::string& eventName, std::vector<std::any> args);

    std::shared_ptr<TzEventEmitterPrivate> d_ptr;
};

namespace utils {

template <typename F>
struct function_traits : function_traits<decltype(&F::operator())> {};

template <typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using args_tuple = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};

template <typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using args_tuple = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
};

template <typename F, std::size_t... Is>
auto callWithAnyVector(F&& f, const std::vector<std::any>& args, std::index_sequence<Is...>)
{
    using traits = function_traits<std::decay_t<F>>;
    return std::invoke(std::forward<F>(f), std::any_cast<std::tuple_element_t<Is, typename traits::args_tuple>>(args[Is])...);
}

template <typename Callable>
auto makeListener(Callable&& cb) -> std::function<void(const std::vector<std::any>&)> {
    using traits = function_traits<std::decay_t<Callable>>;
    return [cb = std::forward<Callable>(cb)](const std::vector<std::any>& args) {
        if (args.size() < traits::arity)
            throw std::invalid_argument("Too few arguments for listener");

        if constexpr (traits::arity == 0) {
            std::invoke(cb);
        } else {
            callWithAnyVector(cb, args, std::make_index_sequence<traits::arity>{});
        }
    };
}

template <typename... Args>
auto packArgs(Args &&... args) -> std::vector<std::any> {
    std::vector<std::any> v;
    v.reserve(sizeof...(Args));
    (v.emplace_back(std::forward<Args>(args)), ...);
    return v;
}

} // namespace utils

template <typename Callable>
inline TzEventListener TzEventEmitter::on(const std::string& eventName, Callable&& cb)
{
    return on(eventName, utils::makeListener(std::forward<Callable>(cb)));
}

template <typename Callable>
inline TzEventListener TzEventEmitter::once(const std::string& eventName, Callable&& cb)
{
    return once(eventName, utils::makeListener(std::forward<Callable>(cb)));
}

template <typename... Args>
inline void TzEventEmitter::emit(const std::string& eventName, Args &&... args)
{
    emit(eventName, utils::packArgs(std::forward<Args>(args)...));
}
