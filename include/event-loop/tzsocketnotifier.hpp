#ifndef TZSOCKETNOTIFIER_HPP
#define TZSOCKETNOTIFIER_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <functional>
#include <memory>

class TzAbstractEventDispatcher;
class TzSocketNotifierPrivate;

class TzSocketNotifier
{
    TZ_DECLARE_PRIVATE(TzSocketNotifier)
public:
    using NotifyHandle = void *;
    using NotifyCallback = std::function<void(int)>;

    explicit TzSocketNotifier(TzAbstractEventDispatcher *dispatcher);
    ~TzSocketNotifier();

    NotifyHandle handle() const;

    void setFd(int fd);
    int fd() const;

    void setCallback(NotifyCallback callback);

    void start();
    void stop();

private:
    std::unique_ptr<TzSocketNotifierPrivate> d_ptr;
};

#endif // TZSOCKETNOTIFIER_HPP
