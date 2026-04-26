#ifndef TZKEYBOARDHANDLER_HPP
#define TZKEYBOARDHANDLER_HPP

#include <event-loop/tzclasshelpermacros.hpp>

#include <functional>
#include <memory>

class TzAbstractEventDispatcher;
class TzAbstractConsoleInput;
class TzKeyEvent;
class TzKeyboardHandlerPrivate;

class TzKeyboardHandler
{
    TZ_DECLARE_PRIVATE(TzKeyboardHandler)
public:
    using KeyCallback = std::function<void(TzKeyEvent *)>;

    explicit TzKeyboardHandler(TzAbstractEventDispatcher *dispatcher, TzAbstractConsoleInput *consoleInput);
    ~TzKeyboardHandler();

    void setCallback(KeyCallback callback);

    void start();
    void stop();

    static TzKeyboardHandler *create(TzAbstractEventDispatcher *dispatcher, TzAbstractConsoleInput *consoleInput, KeyCallback callback);

private:
    std::unique_ptr<TzKeyboardHandlerPrivate> d_ptr;
};

#endif // TZKEYBOARDHANDLER_HPP
