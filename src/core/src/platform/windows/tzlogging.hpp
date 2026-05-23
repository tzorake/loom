#pragma once
#include <ctime>

inline std::tm tzLocalTime(const std::time_t &t)
{
    std::tm tm{};
    localtime_s(&tm, &t);
    return tm;
}
