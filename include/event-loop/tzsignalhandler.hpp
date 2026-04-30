#ifndef TZSIGNALHANDLER_HPP
#define TZSIGNALHANDLER_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <functional>
#include <memory>

class TzAbstractEventDispatcher;
class TzSignalHandlerPrivate;

class TzSignalHandler
{
    TZ_DECLARE_PRIVATE(TzSignalHandler)
public:
    using SignalCallback = std::function<void(int signo)>;

    explicit TzSignalHandler(TzAbstractEventDispatcher *eventDispatcher);
    ~TzSignalHandler();

    void setSignal(int signo);
    void setCallback(SignalCallback callback);

    void start();
    void stop();

    static TzSignalHandler *create(TzAbstractEventDispatcher *eventDispatcher, int signo, SignalCallback callback);

private:
    std::unique_ptr<TzSignalHandlerPrivate> d_ptr;
};

#endif // TZSIGNALHANDLER_HPP
