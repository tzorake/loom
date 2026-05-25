#ifndef TZSIGNALHANDLER_HPP
#define TZSIGNALHANDLER_HPP

#include <loom/tzclasshelpermacros.hpp>

#include <functional>
#include <memory>

class TzAbstractEventDispatcher;
class TzSignalHandlerPrivate;

class TzSignalHandler
{
    TZ_DECLARE_PRIVATE(TzSignalHandler)
public:
    using SignalCallback = std::function<void(int signo)>;

    TzSignalHandler();
    ~TzSignalHandler();

    void setSignal(int signo);
    void setCallback(SignalCallback callback);

    void start();
    void stop();

    static TzSignalHandler *create(int signo, SignalCallback callback);

private:
    std::unique_ptr<TzSignalHandlerPrivate> d_ptr;
};

#endif // TZSIGNALHANDLER_HPP
