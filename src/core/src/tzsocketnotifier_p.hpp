#ifndef TZSOCKETNOTIFIER_P_HPP
#define TZSOCKETNOTIFIER_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzsocketnotifier.hpp>

class TzAbstractEventDispatcher;

class TzSocketNotifierPrivate
{
    TZ_DECLARE_PUBLIC(TzSocketNotifier)
public:
    explicit TzSocketNotifierPrivate();

private:
    TzSocketNotifier *q_ptr{nullptr};

    TzSocketNotifier::NotifyHandle handle{nullptr};
    int fd{-1};
    TzSocketNotifier::NotifyCallback callback;
};

#endif // TZSOCKETNOTIFIER_P_HPP
