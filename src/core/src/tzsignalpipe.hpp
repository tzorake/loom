#ifndef TZSIGNALPIPE_HPP
#define TZSIGNALPIPE_HPP

class TzSignalPipe
{
public:
    TzSignalPipe();   // creates the pipe, throws std::runtime_error on failure
    ~TzSignalPipe();

    TzSignalPipe(const TzSignalPipe &) = delete;
    TzSignalPipe &operator=(const TzSignalPipe &) = delete;

    int readFd()  const { return m_fds[0]; }
    int writeFd() const { return m_fds[1]; }

    int  read(void *buf, int count);
    int  write(const void *buf, int count);
    void makeWriteNonBlocking();

private:
    int m_fds[2] { -1, -1 };
};

#endif // TZSIGNALPIPE_HPP
