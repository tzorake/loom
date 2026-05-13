#include "../../tzsignalpipe.hpp"

#include <io.h>
#include <stdexcept>

TzSignalPipe::TzSignalPipe()
{
    if (_pipe(m_fds, 4096, 0) == -1)
        throw std::runtime_error("TzSignalPipe: _pipe() failed");
}

TzSignalPipe::~TzSignalPipe()
{
    if (m_fds[0] != -1) _close(m_fds[0]);
    if (m_fds[1] != -1) _close(m_fds[1]);
}

int TzSignalPipe::read(void *buf, int count)
{
    return _read(m_fds[0], buf, count);
}

int TzSignalPipe::write(const void *buf, int count)
{
    return _write(m_fds[1], buf, count);
}

void TzSignalPipe::makeWriteNonBlocking()
{
    // Windows CRT pipes used only for signal delivery; no non-blocking flag needed.
}
