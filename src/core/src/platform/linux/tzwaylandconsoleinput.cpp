#include "tzwaylandconsoleinput.hpp"
#include "tzwaylandconsoleinput_p.hpp"

#include <unistd.h>
#include <stdexcept>
#include <cerrno>

TzWaylandConsoleInput::TzWaylandConsoleInput()
    : d_ptr(new TzWaylandConsoleInputPrivate)
{
}

TzWaylandConsoleInput::~TzWaylandConsoleInput()
{
    if (d_ptr->rawActive)
        stop();
}

int TzWaylandConsoleInput::fd() const
{
    return STDIN_FILENO;
}

void TzWaylandConsoleInput::start()
{
    if (!isatty(STDIN_FILENO)) {
        // stdin is not a terminal (e.g. pipe or redirect); raw mode is unavailable.
        // read() will still work in cooked/line-buffered mode.
        return;
    }

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

void TzWaylandConsoleInput::stop()
{
    if (!d_ptr->rawActive)
        return;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &d_ptr->origTermios);
    d_ptr->rawActive = false;
}

std::string TzWaylandConsoleInput::read()
{
    char buf[16];
    ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0)
        return {};
    return std::string(buf, static_cast<size_t>(n));
}
