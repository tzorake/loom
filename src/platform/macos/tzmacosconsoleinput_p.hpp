#ifndef TZMACOSCONSOLEINPUT_P_HPP
#define TZMACOSCONSOLEINPUT_P_HPP

#include <event-loop/tzclasshelpermacros.hpp>
#include <event-loop/tzmacosconsoleinput.hpp>

#include <termios.h>

class TzMacosConsoleInputPrivate
{
public:
    struct termios origTermios;
    struct termios rawTermios;
    bool rawActive{ false };
};

#endif // TZMACOSCONSOLEINPUT_P_HPP
