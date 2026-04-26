#ifndef TZPOSIXKEYBOARDHANDLER_P_HPP
#define TZPOSIXKEYBOARDHANDLER_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzkeyboardhandler.hpp>

#include <termios.h>
#include <cstring>

class TzAbstractEventDispatcher;

class TzPosixKeyboardHandlerPrivate : public TzKeyboardHandlerPrivate
{
    TZ_DECLARE_PUBLIC(TzKeyboardHandler)
public:
    explicit TzPosixKeyboardHandlerPrivate(TzAbstractEventDispatcher *dispatcher);

    virtual void start() override;
    virtual void stop() override;

    virtual void onInputAvailable() override;

    struct termios origTermios;
    struct termios rawTermios;
};

#endif // TZPOSIXKEYBOARDHANDLER_P_HPP
