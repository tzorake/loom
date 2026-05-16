#include <models/tzeventemitter.h>
#include <tzeventemitter_p.h>
#include <models/tzeventlistener.h>
#include <models/tzscopedeventlistener.h>

TzEventListener::TzEventListener()
{
}

TzEventListener::~TzEventListener()
{
}

TzEventListener::TzEventListener(std::weak_ptr<TzEventEmitterPrivate> impl, std::string event, std::uint64_t id)
    : m_impl(std::move(impl))
    , m_event(std::move(event))
    , m_id(id)
{
}

void TzEventListener::disconnect()
{
    auto sp = m_impl.lock();
    if (!sp)
        return;
    auto it = sp->listeners.find(m_event);
    if (it != sp->listeners.end()) {
        auto& lst = it->second;
        lst.remove_if([id = m_id](const auto& pair) { return pair.first == id; });
        if (lst.empty()) sp->listeners.erase(it);
    }
    m_impl.reset();
}

bool TzEventListener::isConnected() const
{
    auto sp = m_impl.lock();
    if (!sp)
        return false;
    auto it = sp->listeners.find(m_event);
    if (it == sp->listeners.end())
        return false;
    for (const auto& pair : it->second)
        if (pair.first == m_id)
            return true;
    return false;
}

void TzEventListener::swap(TzEventListener& other) noexcept
{
    std::swap(m_impl, other.m_impl);
    std::swap(m_event, other.m_event);
    std::swap(m_id, other.m_id);
}

TzEventListener::operator bool() const
{
    return isConnected();
}

TzScopedEventListener::TzScopedEventListener()
{
}

TzScopedEventListener::TzScopedEventListener(TzEventListener listener)
    : m_listener(std::move(listener))
{}

TzScopedEventListener::TzScopedEventListener(TzScopedEventListener&& other) noexcept
{
    swap(other);
}

TzScopedEventListener& TzScopedEventListener::operator=(TzScopedEventListener&& other) noexcept
{
    if (this != &other)
        TzScopedEventListener(std::move(other)).swap(*this);
    return *this;
}

TzScopedEventListener::~TzScopedEventListener()
{
    m_listener.disconnect();
}

void TzScopedEventListener::swap(TzScopedEventListener& other) noexcept
{
    std::swap(m_listener, other.m_listener);
}

TzScopedEventListener::operator bool() const
{
    return m_listener.isConnected();
}

TzEventListener TzScopedEventListener::release() noexcept
{
    auto old = std::move(m_listener);
    m_listener = TzEventListener{};
    return old;
}

TzEventEmitter::TzEventEmitter()
    : d_ptr(new TzEventEmitterPrivate)
{
}

TzEventEmitter::~TzEventEmitter()
{
}

TzEventListener TzEventEmitter::on(const std::string& eventName, ListenerCallback callback)
{
    auto [id, event] = d_ptr->addListener(eventName, std::move(callback));
    return TzEventListener(d_ptr, event, id);
}

TzEventListener TzEventEmitter::once(const std::string& eventName, ListenerCallback callback)
{
    auto [id, evt] = d_ptr->addOnceListener(eventName, std::move(callback));
    return TzEventListener(d_ptr, evt, id);
}

bool TzEventEmitter::removeListener(const TzEventListener& listener)
{
    return d_ptr->removeListener(listener);
}

void TzEventEmitter::removeAllListeners(const std::string& eventName)
{
    d_ptr->removeAllListeners(eventName);
}

void TzEventEmitter::emit(const std::string& eventName, std::vector<std::any> args)
{
    d_ptr->emitPacked(eventName, args);
}

std::vector<std::string> TzEventEmitter::eventNames() const
{
    return d_ptr->eventNames();
}

std::size_t TzEventEmitter::listenerCount(const std::string& eventName) const
{
    return d_ptr->listenerCount(eventName);
}

void TzEventEmitter::setMaxListeners(std::size_t n)
{
    d_ptr->setMaxListeners(n);
}

std::size_t TzEventEmitter::getMaxListeners() const
{
    return d_ptr->getMaxListeners();
}

void TzEventEmitter::setDefaultMaxListeners(std::size_t n)
{
    TzEventEmitterPrivate::defaultMaxListeners = n;
}

std::size_t TzEventEmitter::getDefaultMaxListeners()
{
    return TzEventEmitterPrivate::defaultMaxListeners;
}

TzEventEmitterPrivate::TzEventEmitterPrivate()
{
}

void TzEventEmitterPrivate::setMaxListeners(std::size_t n)
{
    maxListeners = n;
}

std::size_t TzEventEmitterPrivate::getMaxListeners() const
{
    return maxListeners;
}

std::pair<std::uint64_t, std::string> TzEventEmitterPrivate::addListener(const std::string& eventName, ListenerCallback callback)
{
    std::uint64_t id = nextId++;
    auto& lst = listeners[eventName];
    lst.emplace_back(id, std::move(callback));
    checkLimit(eventName, lst.size());
    return { id, eventName };
}

std::pair<std::uint64_t, std::string> TzEventEmitterPrivate::addOnceListener(const std::string& eventName, ListenerCallback callback)
{
    std::uint64_t id = nextId++;
    auto onceFn = [weakImpl = weak_from_this(), eventName, id, callback = std::move(callback)](const std::vector<std::any>& args)
        {
            callback(args);
            if (auto sp = weakImpl.lock()) {
                auto it = sp->listeners.find(eventName);
                if (it != sp->listeners.end()) {
                    auto& lst = it->second;
                    lst.remove_if([id](const auto& pair) { return pair.first == id; });
                    if (lst.empty()) sp->listeners.erase(it);
                }
            }
        };

    auto& lst = listeners[eventName];
    lst.emplace_back(id, std::move(onceFn));
    checkLimit(eventName, lst.size());
    return { id, eventName };
}

bool TzEventEmitterPrivate::removeListener(const TzEventListener& listener)
{
    if (!listener.isConnected())
        return false;
    const_cast<TzEventListener&>(listener).disconnect();
    return true;
}

void TzEventEmitterPrivate::removeAllListeners(const std::string& eventName)
{
    if (eventName.empty()) {
        listeners.clear();
    } else {
        listeners.erase(eventName);
    }
}

void TzEventEmitterPrivate::emitPacked(const std::string& eventName, const std::vector<std::any>& args)
{
    auto it = listeners.find(eventName);
    if (it == listeners.end() || it->second.empty()) {
        if (eventName == "error") {
            if (!args.empty()) {
                if (auto* eptr = std::any_cast<std::exception_ptr>(&args[0]))
                    std::rethrow_exception(*eptr);
                if (auto* msg = std::any_cast<std::string>(&args[0]))
                    throw std::runtime_error(*msg);
            }
            throw std::runtime_error("Unhandled 'error' event");
        }
        return;
    }

    auto listenersCopy = it->second;
    for (const auto& pair : listenersCopy)
        pair.second(args);
}

std::vector<std::string> TzEventEmitterPrivate::eventNames() const
{
    std::vector<std::string> names;
    names.reserve(listeners.size());
    for (const auto& p : listeners)
        names.push_back(p.first);
    return names;
}

std::size_t TzEventEmitterPrivate::listenerCount(const std::string& eventName) const
{
    auto it = listeners.find(eventName);
    return it != listeners.end() ? it->second.size() : 0;
}

void TzEventEmitterPrivate::checkLimit(const std::string& event, std::size_t count)
{
    if (count > maxListeners) {
        std::cerr << std::format(
            "[TzEventEmitter] MaxListenersExceededWarning: "
            "{} listeners on event '{}'. Use setMaxListeners() to increase limit.", count, event) << std::endl;
    }
}
