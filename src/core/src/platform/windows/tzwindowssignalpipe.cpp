#include "../../tzsignalpipe.hpp"

#include <io.h>

namespace TzSignalPipe {

int create(int fds[2])
{
    return _pipe(fds, 4096, 0);
}

void makeNonBlocking(int /*fd*/)
{
    // Windows CRT pipes used only for signal delivery; no non-blocking flag needed.
}

int read(int fd, void *buf, int count)
{
    return _read(fd, buf, count);
}

int write(int fd, const void *buf, int count)
{
    return _write(fd, buf, count);
}

int close(int fd)
{
    return _close(fd);
}

} // namespace TzSignalPipe
