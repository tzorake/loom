#ifndef TZWIN32CONSOLEINPUT_P_HPP
#define TZWIN32CONSOLEINPUT_P_HPP

#include "tzwin32consoleinput.hpp"
#include <loom/tzclasshelpermacros.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class TzWin32ConsoleInputPrivate
{
public:
    HANDLE conHandle{INVALID_HANDLE_VALUE};
    DWORD origMode{0};
    bool rawActive{false};
    bool isConsole{false};
};

#endif // TZWIN32CONSOLEINPUT_P_HPP
