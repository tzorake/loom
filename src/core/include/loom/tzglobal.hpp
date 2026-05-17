#ifndef TZGLOBAL_HPP
#define TZGLOBAL_HPP

#define TZ_UNUSED(x) (void)x;

#define TZ_UNIMPLEMENTED() tzWarning("Unimplemented code.")

constexpr inline void tzNoop(void) noexcept {}

#endif // TZGLOBAL_HPP
