// Include tzbigint.h for declarations only — the implementation is compiled
// in tzbigint.cpp. Pull in the fixed implementation here.
// TZFIXED_IMPLEMENTATION must be defined in exactly one translation unit.
#include "tzbigint.h"
#define TZFIXED_IMPLEMENTATION
#include "tzfixed.h"

#include <loom/tzfixed.hpp>

#include <cstdlib>
#include <new>
#include <stdexcept>
#include <utility>

TzFixed::TzFixed(int fracBits)
    : d_ptr(fixed_create(fracBits))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzFixed::TzFixed(double value, int fracBits)
    : d_ptr(fixed_from_double(value, fracBits))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzFixed::TzFixed(const char *str, int fracBits)
    : d_ptr(fixed_from_string(str, fracBits))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzFixed::TzFixed(const std::string &str, int fracBits)
    : TzFixed(str.data(), fracBits)
{
}

TzFixed::TzFixed(fixed_t &dd) noexcept
    : d_ptr(&dd)
{
}

TzFixed::TzFixed(const TzFixed &other)
    : d_ptr(fixed_clone(other.d_ptr))
{
    if (!d_ptr)
        throw std::bad_alloc();
}

TzFixed::TzFixed(TzFixed &&other) noexcept
    : d_ptr(std::exchange(other.d_ptr, nullptr))
{
}

TzFixed &TzFixed::operator=(const TzFixed &other)
{
    if (this != &other) {
        TzFixed tmp(other);
        swap(tmp);
    }
    return *this;
}

TzFixed &TzFixed::operator=(TzFixed &&other) noexcept
{
    if (this != &other) {
        fixed_destroy(d_ptr);
        d_ptr = std::exchange(other.d_ptr, nullptr);
    }
    return *this;
}

TzFixed::~TzFixed()
{
    fixed_destroy(d_ptr);
}

int TzFixed::fracBits() const
{
    return fixed_get_frac_bits(d_ptr);
}

TzBigInt TzFixed::numerator() const
{
    bigint_t *num = fixed_get_numerator(d_ptr);
    if (!num)
        throw std::runtime_error("TzFixed: no numerator");
    bigint_t *cloned = bigint_clone(num);
    if (!cloned)
        throw std::bad_alloc();
    return TzBigInt(*cloned);
}

double TzFixed::toDouble() const
{
    return fixed_to_double(d_ptr);
}

std::string TzFixed::toString() const
{
    char *cstr = fixed_to_string(d_ptr);
    if (!cstr)
        return {};
    std::string result(cstr);
    std::free(cstr);
    return result;
}

TzFixed TzFixed::rescale(int newFracBits) const
{
    fixed_t *p = fixed_rescale(d_ptr, newFracBits);
    if (!p)
        throw std::bad_alloc();
    return TzFixed(*p);
}

TzFixed &TzFixed::operator+=(const TzFixed &other)
{
    if (fixed_add_assign(d_ptr, other.d_ptr) != 0)
        throw std::invalid_argument("TzFixed: fracBits must match for addition");
    return *this;
}

TzFixed &TzFixed::operator-=(const TzFixed &other)
{
    if (fixed_sub_assign(d_ptr, other.d_ptr) != 0)
        throw std::invalid_argument("TzFixed: fracBits must match for subtraction");
    return *this;
}

TzFixed &TzFixed::operator*=(const TzFixed &other)
{
    if (fixed_mul_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzFixed: multiplication failed");
    return *this;
}

TzFixed &TzFixed::operator/=(const TzFixed &other)
{
    if (fixed_div_assign(d_ptr, other.d_ptr) != 0)
        throw std::runtime_error("TzFixed: division failed");
    return *this;
}

bool TzFixed::operator==(const TzFixed &other) const
{
    return fixed_cmp(d_ptr, other.d_ptr) == 0;
}

bool TzFixed::operator!=(const TzFixed &other) const
{
    return fixed_cmp(d_ptr, other.d_ptr) != 0;
}

bool TzFixed::operator<(const TzFixed &other) const
{
    return fixed_cmp(d_ptr, other.d_ptr) < 0;
}

bool TzFixed::operator>(const TzFixed &other) const
{
    return fixed_cmp(d_ptr, other.d_ptr) > 0;
}

bool TzFixed::operator<=(const TzFixed &other) const
{
    return fixed_cmp(d_ptr, other.d_ptr) <= 0;
}

bool TzFixed::operator>=(const TzFixed &other) const
{
    return fixed_cmp(d_ptr, other.d_ptr) >= 0;
}

void TzFixed::swap(TzFixed &other) noexcept
{
    std::swap(d_ptr, other.d_ptr);
}
