#ifndef TZMACOSCONSOLEINPUT_P_HPP
#define TZMACOSCONSOLEINPUT_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include "tzmacosconsoleinput.hpp"

#include <termios.h>

class TzMacosConsoleInputPrivate
{
public:
    struct termios origTermios;
    struct termios rawTermios;
    bool rawActive{ false };
};

#endif // TZMACOSCONSOLEINPUT_P_HPP
