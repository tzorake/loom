#ifndef TZGLOBAL_HPP
#define TZGLOBAL_HPP

#include <loom/tzlogging.hpp>
#include <loom/tzflags.hpp>

#define TZ_UNUSED(x) (void)x;

enum class TzOrientation {
    Horizontal,
    Vertical
};

enum class TzSortOrder {
    AscendingOrder,
    DescendingOrder
};

enum TzItemDataRole {
    DisplayRole = 0,
    EditRole = 2,
    UserRole = 0x0100
};

enum TzItemFlag {
    NoItemFlags = 0,
    ItemIsSelectable = 1,
    ItemIsEditable = 2,
    ItemIsDragEnabled = 4,
    ItemIsDropEnabled = 8,
    ItemIsUserCheckable = 16,
    ItemIsEnabled = 32,
    ItemIsAutoTristate = 64,
    ItemNeverHasChildren = 128,
    ItemIsUserTristate = 256
};
TZ_DECLARE_FLAGS(TzItemFlags, TzItemFlag)
TZ_DECLARE_OPERATORS_FOR_FLAGS(TzItemFlags)

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
