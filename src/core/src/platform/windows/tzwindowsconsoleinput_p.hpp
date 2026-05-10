#ifndef TZWINDOWSCONSOLEINPUT_P_HPP
#define TZWINDOWSCONSOLEINPUT_P_HPP

#include <loom/tzclasshelpermacros.hpp>
#include "tzwindowsconsoleinput.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class TzWindowsConsoleInputPrivate
{
public:
    HANDLE conHandle{ INVALID_HANDLE_VALUE };
    DWORD origMode{ 0 };
    bool rawActive{ false };
    bool isConsole{ false };
};

#endif // TZWINDOWSCONSOLEINPUT_P_HPP
