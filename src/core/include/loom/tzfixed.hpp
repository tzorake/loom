#ifndef TZFIXED_HPP
#define TZFIXED_HPP

#include <loom/tzbigint.hpp>

#include <string>

struct fixed_t;

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

private:
    explicit TzFixed(fixed_t &dd) noexcept;

    fixed_t *d_ptr;
};

#endif // TZFIXED_HPP
