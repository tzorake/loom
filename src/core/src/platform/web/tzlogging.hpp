#pragma once
#include <ctime>

// WASI provides gmtime_r.  localtime_r is also available on recent WASI SDK
// releases, but timezone data is not populated in the browser, so UTC is the
// most reliable choice.
inline std::tm tzLocalTime(const std::time_t &t)
{
    std::tm tm{};
    gmtime_r(&t, &tm);
    return tm;
}
