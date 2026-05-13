#include "../../tzsignalpipe.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>

TzSignalPipe::TzSignalPipe()
{
    if (::pipe(m_fds) == -1)
        throw std::runtime_error("TzSignalPipe: pipe() failed");
}

TzSignalPipe::~TzSignalPipe()
{
    if (m_fds[0] != -1) ::close(m_fds[0]);
    if (m_fds[1] != -1) ::close(m_fds[1]);
}

int TzSignalPipe::read(void *buf, int count)
{
    return static_cast<int>(::read(m_fds[0], buf, static_cast<size_t>(count)));
}

int TzSignalPipe::write(const void *buf, int count)
{
    return static_cast<int>(::write(m_fds[1], buf, static_cast<size_t>(count)));
}

void TzSignalPipe::makeWriteNonBlocking()
{
    fcntl(m_fds[1], F_SETFL, O_NONBLOCK);
}
