#include "tzx11consoleinput.hpp"
#include "tzx11consoleinput_p.hpp"

#include <stdexcept>
#include <unistd.h>

TzX11ConsoleInput::TzX11ConsoleInput()
    : d_ptr(new TzX11ConsoleInputPrivate)
{}

TzX11ConsoleInput::~TzX11ConsoleInput()
{
    if (d_ptr->rawActive)
        stop();
}

int TzX11ConsoleInput::fd() const
{
    return STDIN_FILENO;
}

void TzX11ConsoleInput::start()
{
    if (!isatty(STDIN_FILENO))
        return;

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

void TzX11ConsoleInput::stop()
{
    if (!d_ptr->rawActive)
        return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &d_ptr->origTermios);
    d_ptr->rawActive = false;
}

std::string TzX11ConsoleInput::read()
{
    char buf[16];
    ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0)
        return {};
    return std::string(buf, static_cast<size_t>(n));
}
