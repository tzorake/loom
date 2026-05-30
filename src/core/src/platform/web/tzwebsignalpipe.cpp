#include "../../tzsignalpipe.hpp"

// Web platform — no-op signal pipe.
//
// The browser has no OS-level signal delivery mechanism.  This stub satisfies
// TzSignalPipe's constructor contract without actually creating a pipe, so
// TzSignalHandler (and therefore TzCoreApplication) can be constructed without
// throwing.
//
// Because the pipe fds are left at -1, the TzSocketNotifier that TzSignalHandler
// registers uses fd -1.  Our web event dispatcher's registerSocketNotifier()
// is also a no-op stub, so the whole chain silently does nothing — which is
// the correct behaviour on web (no SIGINT to handle).

TzSignalPipe::TzSignalPipe()
{
    // m_fds stay at their initialised value of {-1, -1}.
}

TzSignalPipe::~TzSignalPipe()
{
    // Nothing to close.
}

int TzSignalPipe::read(void * /*buf*/, int /*count*/)
{
    return 0;
}

int TzSignalPipe::write(const void * /*buf*/, int /*count*/)
{
    return 0;
}

void TzSignalPipe::makeWriteNonBlocking()
{
    // No-op.
}
