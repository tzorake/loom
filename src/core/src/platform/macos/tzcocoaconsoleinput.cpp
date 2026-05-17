#include "tzcocoaconsoleinput.hpp"

#include "tzcocoaconsoleinput_p.hpp"

#include <stdexcept>
#include <unistd.h>

TzCocoaConsoleInput::TzCocoaConsoleInput()
    : d_ptr(new TzCocoaConsoleInputPrivate)
{}

TzCocoaConsoleInput::~TzCocoaConsoleInput()
{
    if (d_ptr->rawActive)
        stop();
}

int TzCocoaConsoleInput::fd() const
{
    return STDIN_FILENO;
}

void TzCocoaConsoleInput::start()
{
    if (tcgetattr(STDIN_FILENO, &d_ptr->origTermios) == -1)
        throw std::runtime_error("tcgetattr failed");

    d_ptr->rawTermios = d_ptr->origTermios;
    cfmakeraw(&d_ptr->rawTermios);
    d_ptr->rawTermios.c_lflag &= ~(ECHO | ICANON);
    d_ptr->rawTermios.c_oflag |= OPOST;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &d_ptr->rawTermios) == -1)
        throw std::runtime_error("tcsetattr failed");

    d_ptr->rawActive = true;
}

void TzCocoaConsoleInput::stop()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &d_ptr->origTermios);
    d_ptr->rawActive = false;
}

std::string TzCocoaConsoleInput::read()
{
    char buf[16];
    ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0)
        return {};
    return std::string(buf, n);
}
