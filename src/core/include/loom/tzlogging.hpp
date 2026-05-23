#pragma once
#include <loom/tzclasshelpermacros.hpp>
#include <cstdio>
#include <format>
#include <functional>
#include <string>

enum class TzLogLevel { Trace, Debug, Info, Warning, Error, Fatal };

struct TzLogRecord
{
    TzLogLevel level;
    std::string message;
    const char *file;
    int line;
};

using TzLogHandler = std::function<void(const TzLogRecord &)>;

struct TzMessageLoggerPrivate;

class TzMessageLogger
{
    TZ_DECLARE_PRIVATE(TzMessageLogger)
public:
    static TzMessageLogger &instance();

    ~TzMessageLogger();

    void setLevel(TzLogLevel level);
    TzLogLevel level() const;

    void setQuiet(bool quiet);

    int addHandler(TzLogHandler handler, TzLogLevel minLevel = TzLogLevel::Trace);
    void removeHandler(int handle);

    int addFile(std::FILE *file, TzLogLevel minLevel = TzLogLevel::Trace);

    void log(TzLogLevel level, const char *file, int line, std::string message);

private:
    TzMessageLogger();
    
    std::unique_ptr<TzMessageLoggerPrivate> d_ptr;
};

namespace TzLog {

inline void setLevel(TzLogLevel level)
{ TzMessageLogger::instance().setLevel(level); }

inline void setQuiet(bool quiet)
{ TzMessageLogger::instance().setQuiet(quiet); }

inline int addHandler(TzLogHandler h, TzLogLevel min = TzLogLevel::Trace)
{ return TzMessageLogger::instance().addHandler(std::move(h), min); }

inline void removeHandler(int handle)
{ TzMessageLogger::instance().removeHandler(handle); }

inline int addFile(std::FILE *f, TzLogLevel min = TzLogLevel::Trace)
{ return TzMessageLogger::instance().addFile(f, min); }

} // namespace TzLog

#define tzLog(level, fmt, ...) \
    TzMessageLogger::instance().log(level, __FILE__, __LINE__, std::format(fmt __VA_OPT__(,) __VA_ARGS__))

#define tzTrace(...) tzLog(TzLogLevel::Trace, __VA_ARGS__)
#define tzDebug(...) tzLog(TzLogLevel::Debug, __VA_ARGS__)
#define tzInfo(...) tzLog(TzLogLevel::Info, __VA_ARGS__)
#define tzWarning(...) tzLog(TzLogLevel::Warning, __VA_ARGS__)
#define tzError(...) tzLog(TzLogLevel::Error, __VA_ARGS__)
#define tzFatal(...) tzLog(TzLogLevel::Fatal, __VA_ARGS__)
