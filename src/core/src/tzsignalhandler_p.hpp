#ifndef TZSIGNALHANDLER_P_HPP
#define TZSIGNALHANDLER_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzsignalhandler.hpp>

#include "tzsignalpipe.hpp"

#include <memory>

class TzAbstractEventDispatcher;
class TzSocketNotifier;

class TzSignalHandlerPrivate
{
    TZ_DECLARE_PUBLIC(TzSignalHandler)
public:
    TzSignalHandlerPrivate();
    ~TzSignalHandlerPrivate();

    TzSignalHandler *q_ptr{nullptr};

    TzSignalHandler::SignalCallback callback;
    std::unique_ptr<TzSocketNotifier> notifier;

    int signo{-1};
    std::unique_ptr<TzSignalPipe> pipe;
    bool active{false};
};

#endif // TZSIGNALHANDLER_P_HPP
