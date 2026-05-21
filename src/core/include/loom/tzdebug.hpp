#ifndef TZDEBUG_HPP
#define TZDEBUG_HPP

#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

// ---------------------------------------------------------------------------
// TzDebugStream — a RAII stream that flushes a formatted line to stderr on
// destruction.  Auto-inserts a space between each streamed item.
//
// Typical use:
//   tzDebug()   << "Row count:" << 42;
//   tzWarning() << "something went wrong in" << __func__;
//   tzWarning("Unimplemented code.");  // direct single-message form
// ---------------------------------------------------------------------------

class TzDebugStream
{
public:
    enum class Level { Debug, Info, Warning, Critical, Fatal };

    explicit TzDebugStream(Level level);

    // Direct-message constructor (used by tzWarning("msg"), etc.)
    explicit TzDebugStream(Level level, const char *msg);

    TzDebugStream(TzDebugStream &&other) noexcept;
    TzDebugStream(const TzDebugStream &) = delete;
    TzDebugStream &operator=(const TzDebugStream &) = delete;

    ~TzDebugStream();

    // Stream operators
    TzDebugStream &operator<<(bool v);
    TzDebugStream &operator<<(char v);
    TzDebugStream &operator<<(signed char v);
    TzDebugStream &operator<<(unsigned char v);
    TzDebugStream &operator<<(short v);
    TzDebugStream &operator<<(unsigned short v);
    TzDebugStream &operator<<(int v);
    TzDebugStream &operator<<(unsigned int v);
    TzDebugStream &operator<<(long v);
    TzDebugStream &operator<<(unsigned long v);
    TzDebugStream &operator<<(long long v);
    TzDebugStream &operator<<(unsigned long long v);
    TzDebugStream &operator<<(float v);
    TzDebugStream &operator<<(double v);
    TzDebugStream &operator<<(long double v);
    TzDebugStream &operator<<(const char *v);
    TzDebugStream &operator<<(const std::string &v);
    TzDebugStream &operator<<(std::string_view v);
    TzDebugStream &operator<<(const void *v);

    // Control auto-spacing
    TzDebugStream &space();
    TzDebugStream &nospace();
    TzDebugStream &maybeSpace();

private:
    struct Data
    {
        Level         level;
        std::ostringstream buf;
        bool          autoSpace = true;
        bool          needSpace = false;
        bool          flushed   = false;
    };

    Data *d;

    template <typename T>
    TzDebugStream &put(const T &val)
    {
        if (d->autoSpace && d->needSpace)
            d->buf << ' ';
        d->buf << val;
        d->needSpace = true;
        return *this;
    }

    void flush();
};

// ---------------------------------------------------------------------------
// Factory functions — no-argument (stream) form
// ---------------------------------------------------------------------------

TzDebugStream tzDebug();
TzDebugStream tzInfo();
TzDebugStream tzWarning();
TzDebugStream tzCritical();

// ---------------------------------------------------------------------------
// Factory functions — direct single-message form
// ---------------------------------------------------------------------------

TzDebugStream tzDebug(const char *msg);
TzDebugStream tzInfo(const char *msg);
TzDebugStream tzWarning(const char *msg);
TzDebugStream tzCritical(const char *msg);

[[noreturn]] void tzFatal(const char *msg);

#endif // TZDEBUG_HPP
