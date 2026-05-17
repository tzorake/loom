#ifndef TZX11CONSOLEINPUT_P_HPP
#define TZX11CONSOLEINPUT_P_HPP

#include "tzx11consoleinput.hpp"

#include <termios.h>

class TzX11ConsoleInputPrivate
{
public:
    struct termios origTermios;
    struct termios rawTermios;
    bool rawActive{false};
};

#endif // TZX11CONSOLEINPUT_P_HPP
