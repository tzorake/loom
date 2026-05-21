#ifndef TZGLOBAL_HPP
#define TZGLOBAL_HPP

#include <loom/tzdebug.hpp>

#define TZ_UNUSED(x) (void)x;

enum class TzOrientation {
    Horizontal,
    Vertical
};

enum class TzSortOrder {
    AscendingOrder,
    DescendingOrder
};

#define TZ_UNIMPLEMENTED() tzWarning("Unimplemented code.")

constexpr inline void tzNoop(void) noexcept {}

template <typename T>
constexpr inline void tzPtrSwap(T* &lhs, T* &rhs) noexcept
{
    T *tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}

#endif // TZGLOBAL_HPP
