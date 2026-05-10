#ifndef TZNUMERIC_HPP
#define TZNUMERIC_HPP

#include <algorithm>

[[nodiscard]] constexpr double tzAbs(double v) noexcept
{
    return v < 0.0 ? -v : v;
}

[[nodiscard]] constexpr bool tzFuzzyCompare(double p1, double p2) noexcept
{
    return (tzAbs(p1 - p2) * 1000000000000. <= std::min(tzAbs(p1), tzAbs(p2)));
}

#endif // TZNUMERIC_HPP
