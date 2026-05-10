#ifndef TZKEYBOARDHANDLER_HPP
#define TZKEYBOARDHANDLER_HPP

#include <loom/tzclasshelpermacros.hpp>
#include <loom/tzobject.hpp>

#include <functional>
#include <memory>

class TzAbstractEventDispatcher;
class TzAbstractConsoleInput;
class TzKeyEvent;
class TzKeyboardHandlerPrivate;

class TzKeyboardHandler : public TzObject
{
    TZ_DECLARE_PRIVATE(TzKeyboardHandler)
public:
    using KeyCallback = std::function<void(TzKeyEvent *)>;

    explicit TzKeyboardHandler(TzAbstractEventDispatcher *eventDispatcher,
                               TzAbstractConsoleInput *consoleInput,
                               TzObject *parent = nullptr);
    ~TzKeyboardHandler() override;

    void setCallback(KeyCallback callback);

    void start();
    void stop();

    bool event(TzEvent *event) override;

    static TzKeyboardHandler *create(TzAbstractEventDispatcher *eventDispatcher,
                                     TzAbstractConsoleInput *consoleInput,
                                     KeyCallback callback,
                                     TzObject *parent = nullptr);

private:
    std::unique_ptr<TzKeyboardHandlerPrivate> d_ptr;
};

#endif // TZKEYBOARDHANDLER_HPP
