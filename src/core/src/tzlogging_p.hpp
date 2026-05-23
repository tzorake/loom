#pragma once
#include <loom/tzlogging.hpp>
#include <mutex>
#include <vector>

struct TzMessageLoggerPrivate
{
    struct Entry
    {
        int handle;
        TzLogLevel minLevel;
        TzLogHandler handler;
    };

    mutable std::mutex mutex;
    TzLogLevel level{TzLogLevel::Trace};
    bool quiet{false};
    int nextHandle{1};
    std::vector<Entry> handlers;
};
