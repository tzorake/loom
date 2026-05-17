#ifndef TZCOCOACONSOLEINPUT_P_HPP
#define TZCOCOACONSOLEINPUT_P_HPP

#include "tzcocoaconsoleinput.hpp"
#include <loom/tzclasshelpermacros.hpp>

#include <termios.h>

class TzCocoaConsoleInputPrivate
{
public:
    struct termios origTermios;
    struct termios rawTermios;
    bool rawActive{false};
};

#endif // TZCOCOACONSOLEINPUT_P_HPP
