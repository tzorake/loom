#ifndef TZSIGNALPIPE_HPP
#define TZSIGNALPIPE_HPP

namespace TzSignalPipe {
    int  create(int fds[2]);
    void makeNonBlocking(int writeFd);
    int  read(int fd, void *buf, int count);
    int  write(int fd, const void *buf, int count);
    int  close(int fd);
}

#endif // TZSIGNALPIPE_HPP
