#ifndef TZBIGINT_HPP
#define TZBIGINT_HPP

#include <cstddef>
#include <cstdint>
#include <string>

struct bigint_t;

class TzFixed;

class TzBigInt
{
public:
    TzBigInt();
    explicit TzBigInt(int64_t value);
    explicit TzBigInt(const char *str);
    explicit TzBigInt(const std::string &str);

    TzBigInt(const TzBigInt &other);
    TzBigInt(TzBigInt &&other) noexcept;
    TzBigInt &operator=(const TzBigInt &other);
    TzBigInt &operator=(TzBigInt &&other) noexcept;
    ~TzBigInt();

    bool isZero() const;
    bool isNegative() const;
    void setNegative(bool neg);

    double toDouble() const;
    std::string toString() const;

    TzBigInt &operator+=(const TzBigInt &other);
    TzBigInt &operator-=(const TzBigInt &other);
    TzBigInt &operator*=(const TzBigInt &other);
    TzBigInt &operator/=(const TzBigInt &other);
    TzBigInt &operator%=(const TzBigInt &other);

    TzBigInt &operator<<=(size_t bits);
    TzBigInt &operator>>=(size_t bits);

    bool operator==(const TzBigInt &other) const;
    bool operator!=(const TzBigInt &other) const;
    bool operator<(const TzBigInt &other) const;
    bool operator>(const TzBigInt &other) const;
    bool operator<=(const TzBigInt &other) const;
    bool operator>=(const TzBigInt &other) const;

    friend TzBigInt operator+(TzBigInt a, const TzBigInt &b) { return a += b; }
    friend TzBigInt operator-(TzBigInt a, const TzBigInt &b) { return a -= b; }
    friend TzBigInt operator*(TzBigInt a, const TzBigInt &b) { return a *= b; }
    friend TzBigInt operator/(TzBigInt a, const TzBigInt &b) { return a /= b; }
    friend TzBigInt operator%(TzBigInt a, const TzBigInt &b) { return a %= b; }
    friend TzBigInt operator<<(TzBigInt a, size_t bits) { return a <<= bits; }
    friend TzBigInt operator>>(TzBigInt a, size_t bits) { return a >>= bits; }

    void swap(TzBigInt &other) noexcept;

private:
    friend class TzFixed;
    explicit TzBigInt(bigint_t &dd) noexcept;

    bigint_t *d_ptr;
};

#endif // TZBIGINT_HPP
