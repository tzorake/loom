#ifndef TZFIXED_HPP
#define TZFIXED_HPP

#include <loom/tzbigint.hpp>

#include <cmath>
#include <string>

struct fixed_t;

// Forward-declare TzFixed so the std overloads below can name it as a parameter
// type before the full class definition.
class TzFixed;

namespace std {
    TzFixed sin(TzFixed x);
    TzFixed cos(TzFixed x);
    TzFixed sqrt(TzFixed x);
    TzFixed atan2(TzFixed y, TzFixed x);
} // namespace std

class TzFixed
{
public:
    explicit TzFixed(int fracBits);
    TzFixed(double value, int fracBits);
    TzFixed(const char *str, int fracBits);
    TzFixed(const std::string &str, int fracBits);

    TzFixed(const TzFixed &other);
    TzFixed(TzFixed &&other) noexcept;
    TzFixed &operator=(const TzFixed &other);
    TzFixed &operator=(TzFixed &&other) noexcept;
    ~TzFixed();

    int fracBits() const;
    TzBigInt numerator() const;
    double toDouble() const;
    std::string toString() const;

    TzFixed rescale(int newFracBits) const;

    TzFixed &operator+=(const TzFixed &other);
    TzFixed &operator-=(const TzFixed &other);
    TzFixed &operator*=(const TzFixed &other);
    TzFixed &operator/=(const TzFixed &other);

    friend TzFixed operator+(TzFixed a, const TzFixed &b) { return a += b; }
    friend TzFixed operator-(TzFixed a, const TzFixed &b) { return a -= b; }
    friend TzFixed operator*(TzFixed a, const TzFixed &b) { return a *= b; }
    friend TzFixed operator/(TzFixed a, const TzFixed &b) { return a /= b; }

    bool operator==(const TzFixed &other) const;
    bool operator!=(const TzFixed &other) const;
    bool operator<(const TzFixed &other) const;
    bool operator>(const TzFixed &other) const;
    bool operator<=(const TzFixed &other) const;
    bool operator>=(const TzFixed &other) const;

    void swap(TzFixed &other) noexcept;

    bool isNegative() const;
    bool isZero() const;

private:
    explicit TzFixed(fixed_t &dd) noexcept;

    friend TzFixed std::sin(TzFixed);
    friend TzFixed std::cos(TzFixed);
    friend TzFixed std::sqrt(TzFixed);
    friend TzFixed std::atan2(TzFixed, TzFixed);

    fixed_t *d_ptr;
};

#endif // TZFIXED_HPP
