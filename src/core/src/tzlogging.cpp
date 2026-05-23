#include <loom/tzlogging.hpp>
#include <tzlogging_p.hpp>

#if defined(_WIN32)
#  include "platform/windows/tzlogging.hpp"
#else
#  include "platform/posix/tzlogging.hpp"
#endif

#include <chrono>
#include <ctime>

namespace {

const char *getLevelString(TzLogLevel level)
{
    switch (level) {
    case TzLogLevel::Trace: return "TRACE";
    case TzLogLevel::Debug: return "DEBUG";
    case TzLogLevel::Info: return "INFO";
    case TzLogLevel::Warning: return "WARNING";
    case TzLogLevel::Error: return "ERROR";
    case TzLogLevel::Fatal: return "FATAL";
    }
    return "";
}

const char *getLevelColor(TzLogLevel level)
{
    switch (level) {
    case TzLogLevel::Trace: return "\x1b[94m";
    case TzLogLevel::Debug: return "\x1b[36m";
    case TzLogLevel::Info: return "\x1b[32m";
    case TzLogLevel::Warning: return "\x1b[33m";
    case TzLogLevel::Error: return "\x1b[31m";
    case TzLogLevel::Fatal: return "\x1b[35m";
    }
    return "";
}

TzLogHandler makeStderrHandler()
{
    return [](const TzLogRecord &r) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = tzLocalTime(t);
        char timebuf[16];
        std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &tm);
        std::fprintf(stderr, "%s %s%s\x1b[0m \x1b[90m%s:%d:\x1b[0m %s\n",
            timebuf, getLevelColor(r.level), getLevelString(r.level),
            r.file, r.line, r.message.data());
        std::fflush(stderr);
    };
}

TzLogHandler makeFileHandler(std::FILE *file)
{
    return [file](const TzLogRecord &r) {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = tzLocalTime(t);
        char timebuf[32];
        std::strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &tm);
        std::fprintf(file, "%s %s %s:%d: %s\n",
            timebuf, getLevelString(r.level),
            r.file, r.line, r.message.data());
        std::fflush(file);
    };
}

} // namespace

TzMessageLogger &TzMessageLogger::instance()
{
    static TzMessageLogger logger{};
    return logger;
}

TzMessageLogger::~TzMessageLogger()
{
}

void TzMessageLogger::setLevel(TzLogLevel level)
{
    TZ_D(TzMessageLogger);
    std::lock_guard lock(d->mutex);
    d->level = level;
}

TzLogLevel TzMessageLogger::level() const
{
    TZ_D(const TzMessageLogger);
    std::lock_guard lock(d->mutex);
    return d->level;
}

void TzMessageLogger::setQuiet(bool quiet)
{
    TZ_D(TzMessageLogger);
    std::lock_guard lock(d->mutex);
    d->quiet = quiet;
}

int TzMessageLogger::addHandler(TzLogHandler handler, TzLogLevel minLevel)
{
    TZ_D(TzMessageLogger);
    std::lock_guard lock(d->mutex);
    int handle = d->nextHandle++;
    d->handlers.push_back({handle, minLevel, std::move(handler)});
    return handle;
}

void TzMessageLogger::removeHandler(int handle)
{
    TZ_D(TzMessageLogger);
    std::lock_guard lock(d->mutex);
    std::erase_if(d->handlers, [handle](const TzMessageLoggerPrivate::Entry &e) {
        return e.handle == handle;
    });
}

int TzMessageLogger::addFile(std::FILE *file, TzLogLevel minLevel)
{
    return addHandler(makeFileHandler(file), minLevel);
}

void TzMessageLogger::log(TzLogLevel level, const char *file, int line, std::string message)
{
    TZ_D(TzMessageLogger);
    std::lock_guard lock(d->mutex);
    if (d->quiet || level < d->level)
        return;

    TzLogRecord record{level, std::move(message), file, line};
    for (const TzMessageLoggerPrivate::Entry &entry : d->handlers) {
        if (record.level >= entry.minLevel)
            entry.handler(record);
    }
}

TzMessageLogger::TzMessageLogger()
    : d_ptr(new TzMessageLoggerPrivate)
{
    d_ptr->handlers.emplace_back(d_ptr->nextHandle++, TzLogLevel::Trace, makeStderrHandler());
}
