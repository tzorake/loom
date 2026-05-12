#include "../../tzsignalpipe.hpp"

#include <unistd.h>
#include <fcntl.h>

namespace TzSignalPipe {

int create(int fds[2])
{
    return ::pipe(fds);
}

void makeNonBlocking(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int read(int fd, void *buf, int count)
{
    return static_cast<int>(::read(fd, buf, static_cast<size_t>(count)));
}

int write(int fd, const void *buf, int count)
{
    return static_cast<int>(::write(fd, buf, static_cast<size_t>(count)));
}

int close(int fd)
{
    return ::close(fd);
}

} // namespace TzSignalPipe
