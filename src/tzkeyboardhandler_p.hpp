#ifndef TZKEYBOARDHANDLER_P_HPP
#define TZKEYBOARDHANDLER_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzkeyboardhandler.hpp>

class TzAbstractEventDispatcher;
class TzSocketNotifier;

class TzKeyboardHandlerPrivate
{
    TZ_DECLARE_PUBLIC(TzKeyboardHandler)
public:
    explicit TzKeyboardHandlerPrivate(TzAbstractEventDispatcher *dispatcher);
    virtual ~TzKeyboardHandlerPrivate();

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void onInputAvailable() = 0;
    
    void processKeyEvent(TzKeyEvent *event);

    TzKeyboardHandler *q_ptr{ nullptr };

    TzAbstractEventDispatcher *dispatcher{ nullptr };
    TzKeyboardHandler::KeyCallback callback;
    std::unique_ptr<TzSocketNotifier> notifier;
    std::string buffer;
    bool active{ false };
    bool rawActive{ false };
};

#endif // TZKEYBOARDHANDLER_P_HPP
