#ifndef TZKEYBOARDHANDLER_P_HPP
#define TZKEYBOARDHANDLER_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzkeyboardhandler.hpp>

class TzAbstractEventDispatcher;
class TzAbstractConsoleInput;
class TzSocketNotifier;

class TzKeyboardHandlerPrivate
{
    TZ_DECLARE_PUBLIC(TzKeyboardHandler)
public:
    explicit TzKeyboardHandlerPrivate(TzAbstractEventDispatcher *eventDispatcher, TzAbstractConsoleInput *consoleInput);
    ~TzKeyboardHandlerPrivate();

    void onInputAvailable();
    void processKeyEvent(TzKeyEvent *event);

    TzKeyboardHandler *q_ptr{ nullptr };

    TzAbstractEventDispatcher *eventDispatcher{ nullptr };
    TzAbstractConsoleInput *consoleInput{ nullptr };
    TzKeyboardHandler::KeyCallback callback;
    std::unique_ptr<TzSocketNotifier> notifier;
    std::string buffer;
    bool active{ false };
};

#endif // TZKEYBOARDHANDLER_P_HPP
