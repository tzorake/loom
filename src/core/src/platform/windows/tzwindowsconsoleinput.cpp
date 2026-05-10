#include "tzwindowsconsoleinput.hpp"
#include "tzwindowsconsoleinput_p.hpp"

#include <io.h>
#include <stdexcept>
#include <string>

// Translate a Windows virtual key to the VT100/ANSI escape sequence expected
// by TzKeyboardHandler. Returns empty string for keys that produce a regular
// Unicode character (handled via uChar.UnicodeChar instead).
static const char *vkToEscape(WORD vk)
{
    switch (vk) {
        case VK_UP:    return "\x1B[A";
        case VK_DOWN:  return "\x1B[B";
        case VK_RIGHT: return "\x1B[C";
        case VK_LEFT:  return "\x1B[D";
        case VK_HOME:  return "\x1B[1~";
        case VK_END:   return "\x1B[4~";
        case VK_PRIOR: return "\x1B[5~";
        case VK_NEXT:  return "\x1B[6~";
        case VK_F1:    return "\x1B[11~";
        case VK_F2:    return "\x1B[12~";
        case VK_F3:    return "\x1B[13~";
        case VK_F4:    return "\x1B[14~";
        case VK_F5:    return "\x1B[15~";
        case VK_F6:    return "\x1B[17~";
        case VK_F7:    return "\x1B[18~";
        case VK_F8:    return "\x1B[19~";
        case VK_F9:    return "\x1B[20~";
        case VK_F10:   return "\x1B[21~";
        case VK_F11:   return "\x1B[23~";
        case VK_F12:   return "\x1B[24~";
        default:       return nullptr;
    }
}

TzWindowsConsoleInput::TzWindowsConsoleInput()
    : d_ptr(new TzWindowsConsoleInputPrivate)
{
    d_ptr->conHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dummy;
    d_ptr->isConsole = GetConsoleMode(d_ptr->conHandle, &dummy);
}

TzWindowsConsoleInput::~TzWindowsConsoleInput()
{
    if (d_ptr->rawActive)
        stop();
}

int TzWindowsConsoleInput::fd() const
{
    return _fileno(stdin);
}

void TzWindowsConsoleInput::start()
{
    if (!d_ptr->isConsole)
        return;

    GetConsoleMode(d_ptr->conHandle, &d_ptr->origMode);

    DWORD rawMode = d_ptr->origMode
        & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(d_ptr->conHandle, rawMode);

    d_ptr->rawActive = true;
}

void TzWindowsConsoleInput::stop()
{
    if (!d_ptr->rawActive)
        return;
    SetConsoleMode(d_ptr->conHandle, d_ptr->origMode);
    d_ptr->rawActive = false;
}

std::string TzWindowsConsoleInput::read()
{
    if (!d_ptr->isConsole)
        return {};

    INPUT_RECORD records[16];
    DWORD numRead = 0;

    if (!ReadConsoleInputW(d_ptr->conHandle, records, 16, &numRead))
        return {};

    std::string result;

    for (DWORD i = 0; i < numRead; ++i) {
        if (records[i].EventType != KEY_EVENT)
            continue;
        if (!records[i].Event.KeyEvent.bKeyDown)
            continue;

        WORD  vk    = records[i].Event.KeyEvent.wVirtualKeyCode;
        WCHAR wch   = records[i].Event.KeyEvent.uChar.UnicodeChar;
        DWORD count = records[i].Event.KeyEvent.wRepeatCount;

        std::string seq;

        // Skip pure modifier key events
        if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU ||
            vk == VK_LSHIFT || vk == VK_RSHIFT ||
            vk == VK_LCONTROL || vk == VK_RCONTROL ||
            vk == VK_LMENU || vk == VK_RMENU)
            continue;

        if (const char *esc = vkToEscape(vk)) {
            seq = esc;
        } else if (vk == VK_ESCAPE) {
            seq = "\x1B";
        } else if (wch != 0) {
            // Convert Unicode character to UTF-8
            char buf[4];
            int len = WideCharToMultiByte(CP_UTF8, 0, &wch, 1, buf, 4, nullptr, nullptr);
            if (len > 0)
                seq.assign(buf, len);
        }

        for (DWORD r = 0; r < count; ++r)
            result += seq;
    }

    return result;
}
