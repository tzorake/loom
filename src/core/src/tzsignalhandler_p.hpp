#ifndef TZSIGNALHANDLER_P_HPP
#define TZSIGNALHANDLER_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzsignalhandler.hpp>

class TzAbstractEventDispatcher;
class TzSocketNotifier;

class TzSignalHandlerPrivate
{
    TZ_DECLARE_PUBLIC(TzSignalHandler)
public:
    explicit TzSignalHandlerPrivate(TzAbstractEventDispatcher *eventDispatcher);
    ~TzSignalHandlerPrivate() = default;

    TzSignalHandler *q_ptr{ nullptr };

    TzAbstractEventDispatcher *eventDispatcher{ nullptr };
    TzSignalHandler::SignalCallback callback;
    std::unique_ptr<TzSocketNotifier> notifier;

    int signo{ -1 };
    int pipeFds[2]{ -1, -1 };
    bool active{ false };
};

#endif // TZSIGNALHANDLER_P_HPP
