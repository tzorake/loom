#ifndef TZWAYLANDCONSOLEINPUT_P_HPP
#define TZWAYLANDCONSOLEINPUT_P_HPP

#include "tzwaylandconsoleinput.hpp"
#include <loom/tzclasshelpermacros.hpp>

#include <termios.h>

class TzWaylandConsoleInputPrivate
{
public:
    struct termios origTermios;
    struct termios rawTermios;
    bool rawActive{false};
};

#endif // TZWAYLANDCONSOLEINPUT_P_HPP
