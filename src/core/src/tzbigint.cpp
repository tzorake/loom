#define TZBIGINT_IMPLEMENTATION
#include "tzbigint.h"

#include <loom/tzbigint.hpp>

#include <cstdlib>
#include <new>
#include <stdexcept>
#include <utility>

TzBigInt::TzBigInt()
    : d_ptr(bigint_create())
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzBigInt::TzBigInt(int64_t value)
    : d_ptr(bigint_from_int64(value))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzBigInt::TzBigInt(const char *str)
    : d_ptr(bigint_from_string(str))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzBigInt::TzBigInt(const std::string &str)
    : TzBigInt(str.data())
{
}

TzBigInt::TzBigInt(bigint_t &dd) noexcept
    : d_ptr(&dd)
{
}

TzBigInt::TzBigInt(const TzBigInt &other)
    : d_ptr(bigint_clone(other.d_ptr))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzBigInt::TzBigInt(TzBigInt &&other) noexcept
    : d_ptr(std::exchange(other.d_ptr, nullptr))
{
}

TzBigInt &TzBigInt::operator=(const TzBigInt &other)
{
    if (this != &other) {
        TzBigInt tmp(other);
        swap(tmp);
    }
    return *this;
}

TzBigInt &TzBigInt::operator=(TzBigInt &&other) noexcept
{
    if (this != &other) {
        bigint_destroy(d_ptr);
        d_ptr = std::exchange(other.d_ptr, nullptr);
    }
    return *this;
}

TzBigInt::~TzBigInt()
{
    bigint_destroy(d_ptr);
}

bool TzBigInt::isZero() const
{
    return bigint_is_zero(d_ptr) != 0;
}

bool TzBigInt::isNegative() const
{
    return bigint_is_negative(d_ptr) != 0;
}

void TzBigInt::setNegative(bool neg)
{
    bigint_set_negative(d_ptr, neg ? 1 : 0);
}

double TzBigInt::toDouble() const
{
    return bigint_to_double(d_ptr);
}

std::string TzBigInt::toString() const
{
    char *cstr = bigint_to_string(d_ptr);
    if (!cstr)
        return {};
    std::string result(cstr);
    std::free(cstr);
    return result;
}

TzBigInt &TzBigInt::operator+=(const TzBigInt &other)
{
    if (bigint_add_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzBigInt: addition failed");
    return *this;
}

TzBigInt &TzBigInt::operator-=(const TzBigInt &other)
{
    if (bigint_sub_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzBigInt: subtraction failed");
    return *this;
}

TzBigInt &TzBigInt::operator*=(const TzBigInt &other)
{
    if (bigint_mul_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzBigInt: multiplication failed");
    return *this;
}

TzBigInt &TzBigInt::operator/=(const TzBigInt &other)
{
    if (bigint_div_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzBigInt: division failed");
    return *this;
}

TzBigInt &TzBigInt::operator%=(const TzBigInt &other)
{
    if (bigint_mod_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzBigInt: modulo failed");
    return *this;
}

TzBigInt &TzBigInt::operator<<=(size_t bits)
{
    bigint_shl(d_ptr, bits);
    return *this;
}
TzBigInt &TzBigInt::operator>>=(size_t bits)
{
    bigint_shr(d_ptr, bits);
    return *this;
}

bool TzBigInt::operator==(const TzBigInt &other) const
{
    return bigint_cmp(d_ptr, other.d_ptr) == 0;
}

bool TzBigInt::operator!=(const TzBigInt &other) const
{
    return bigint_cmp(d_ptr, other.d_ptr) != 0;
}

bool TzBigInt::operator<(const TzBigInt &other) const
{
    return bigint_cmp(d_ptr, other.d_ptr) < 0;
}

bool TzBigInt::operator>(const TzBigInt &other) const
{
    return bigint_cmp(d_ptr, other.d_ptr) > 0;
}

bool TzBigInt::operator<=(const TzBigInt &other) const
{
    return bigint_cmp(d_ptr, other.d_ptr) <= 0;
}

bool TzBigInt::operator>=(const TzBigInt &other) const
{
    return bigint_cmp(d_ptr, other.d_ptr) >= 0;
}

void TzBigInt::swap(TzBigInt &other) noexcept
{
    std::swap(d_ptr, other.d_ptr);
}
