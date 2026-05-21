#include <loom/tzdebug.hpp>
#include <cstdio>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const char *levelPrefix(TzDebugStream::Level level) noexcept
{
    switch (level) {
    case TzDebugStream::Level::Debug:    return "Debug";
    case TzDebugStream::Level::Info:     return "Info";
    case TzDebugStream::Level::Warning:  return "Warning";
    case TzDebugStream::Level::Critical: return "Critical";
    case TzDebugStream::Level::Fatal:    return "Fatal";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// TzDebugStream
// ---------------------------------------------------------------------------

TzDebugStream::TzDebugStream(Level level)
    : d(new Data{level})
{}

TzDebugStream::TzDebugStream(Level level, const char *msg)
    : d(new Data{level})
{
    if (msg)
        d->buf << msg;
    d->needSpace = (msg && *msg != '\0');
}

TzDebugStream::TzDebugStream(TzDebugStream &&other) noexcept
    : d(other.d)
{
    other.d = nullptr;
}

TzDebugStream::~TzDebugStream()
{
    if (d) {
        flush();
        delete d;
    }
}

void TzDebugStream::flush()
{
    if (!d || d->flushed)
        return;
    d->flushed = true;

    std::string msg = d->buf.str();
    std::fprintf(stderr, "%s: %s\n", levelPrefix(d->level), msg.c_str());
}

// ---------------------------------------------------------------------------
// Spacing control
// ---------------------------------------------------------------------------

TzDebugStream &TzDebugStream::space()
{
    d->autoSpace = true;
    d->buf << ' ';
    d->needSpace = false;
    return *this;
}

TzDebugStream &TzDebugStream::nospace()
{
    d->autoSpace = false;
    return *this;
}

TzDebugStream &TzDebugStream::maybeSpace()
{
    if (d->autoSpace && d->needSpace) {
        d->buf << ' ';
        d->needSpace = false;
    }
    return *this;
}

// ---------------------------------------------------------------------------
// Stream operators
// ---------------------------------------------------------------------------

TzDebugStream &TzDebugStream::operator<<(bool v)
{ return put(v ? "true" : "false"); }

TzDebugStream &TzDebugStream::operator<<(char v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(signed char v)
{ return put(static_cast<int>(v)); }

TzDebugStream &TzDebugStream::operator<<(unsigned char v)
{ return put(static_cast<unsigned int>(v)); }

TzDebugStream &TzDebugStream::operator<<(short v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(unsigned short v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(int v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(unsigned int v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(long v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(unsigned long v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(long long v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(unsigned long long v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(float v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(double v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(long double v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(const char *v)
{ return put(v ? v : "(null)"); }

TzDebugStream &TzDebugStream::operator<<(const std::string &v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(std::string_view v)
{ return put(v); }

TzDebugStream &TzDebugStream::operator<<(const void *v)
{
    if (d->autoSpace && d->needSpace)
        d->buf << ' ';
    d->buf << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(v) << std::dec;
    d->needSpace = true;
    return *this;
}

// ---------------------------------------------------------------------------
// Factory functions
// ---------------------------------------------------------------------------

TzDebugStream tzDebug()    { return TzDebugStream(TzDebugStream::Level::Debug); }
TzDebugStream tzInfo()     { return TzDebugStream(TzDebugStream::Level::Info); }
TzDebugStream tzWarning()  { return TzDebugStream(TzDebugStream::Level::Warning); }
TzDebugStream tzCritical() { return TzDebugStream(TzDebugStream::Level::Critical); }

TzDebugStream tzDebug(const char *msg)    { return TzDebugStream(TzDebugStream::Level::Debug,    msg); }
TzDebugStream tzInfo(const char *msg)     { return TzDebugStream(TzDebugStream::Level::Info,     msg); }
TzDebugStream tzWarning(const char *msg)  { return TzDebugStream(TzDebugStream::Level::Warning,  msg); }
TzDebugStream tzCritical(const char *msg) { return TzDebugStream(TzDebugStream::Level::Critical, msg); }

[[noreturn]] void tzFatal(const char *msg)
{
    std::fprintf(stderr, "Fatal: %s\n", msg ? msg : "(null)");
    std::fflush(stderr);
    std::abort();
}
