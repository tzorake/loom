#ifndef TZNUMERIC_HPP
#define TZNUMERIC_HPP

[[nodiscard]] constexpr bool tzFuzzyCompare(double p1, double p2) noexcept
{
    return (std::abs(p1 - p2) * 1000000000000. <= std::min(std::abs(p1), std::abs(p2)));
}

#endif // TZNUMERIC_HPP
