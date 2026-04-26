#ifndef TZSOCKETNOTIFIER_P_HPP
#define TZSOCKETNOTIFIER_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzsocketnotifier.hpp>

class TzAbstractEventDispatcher;

class TzSocketNotifierPrivate
{
    TZ_DECLARE_PUBLIC(TzSocketNotifier)
public:
    explicit TzSocketNotifierPrivate(TzAbstractEventDispatcher *dispatcher);

private:
    TzSocketNotifier *q_ptr{ nullptr };

    TzAbstractEventDispatcher *dispatcher{ nullptr };
    TzSocketNotifier::NotifyHandle handle{ nullptr };
    int fd{ -1 };
    TzSocketNotifier::NotifyCallback callback;
};

#endif // TZSOCKETNOTIFIER_P_HPP
