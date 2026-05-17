
#ifndef TZFLAGS_HPP
#define TZFLAGS_HPP

#include <algorithm>
#include <cstdint>

template<typename Enum>
class TzFlags;

class TzFlag
{
    std::int32_t i;

public:
    constexpr inline TzFlag(std::int32_t value) noexcept
        : i(value)
    {}
    constexpr inline operator std::int32_t() const noexcept { return i; }

    constexpr inline TzFlag(std::uint32_t value) noexcept
        : i(std::int32_t(value))
    {}
    constexpr inline TzFlag(short value) noexcept
        : i(std::int32_t(value))
    {}
    constexpr inline TzFlag(unsigned short value) noexcept
        : i(std::int32_t(std::uint32_t(value)))
    {}
    constexpr inline operator std::uint32_t() const noexcept { return std::uint32_t(i); }
    constexpr inline TzFlag(long value) noexcept
        : i(std::int32_t(value))
    {}
    constexpr inline TzFlag(unsigned long value) noexcept
        : i(std::int32_t(long(value)))
    {}
};

class QIncompatibleTzFlag
{
    std::int32_t i;

public:
    constexpr inline explicit QIncompatibleTzFlag(std::int32_t i) noexcept;
    constexpr inline operator std::int32_t() const noexcept { return i; }
};

constexpr inline QIncompatibleTzFlag::QIncompatibleTzFlag(std::int32_t value) noexcept
    : i(value)
{}

namespace TzPrivate {

template<typename T>
struct IsTzFlags : std::false_type
{};
template<typename E>
struct IsTzFlags<TzFlags<E>> : std::true_type
{};

template<std::int32_t>
struct QIntegerForSize;
template<>
struct QIntegerForSize<1>
{
    typedef std::uint8_t Unsigned;
    typedef std::int8_t Signed;
};
template<>
struct QIntegerForSize<2>
{
    typedef std::uint16_t Unsigned;
    typedef std::int16_t Signed;
};
template<>
struct QIntegerForSize<4>
{
    typedef std::uint32_t Unsigned;
    typedef std::int32_t Signed;
};
template<>
struct QIntegerForSize<8>
{
    typedef std::uint64_t Unsigned;
    typedef std::int64_t Signed;
};

template<typename Enum>
class TzFlagsStorage
{
    static_assert(sizeof(Enum) <= sizeof(std::uint64_t),
                  "Only enumerations 64 bits or smaller are supported.");
    static_assert((std::is_enum<Enum>::value), "TzFlags is only usable on enumeration types.");

    static constexpr std::size_t IntegerSize = (std::max) (sizeof(Enum), sizeof(std::int32_t));
    using Integers = QIntegerForSize<IntegerSize>;

protected:
    using Int =
        typename std::conditional<std::is_unsigned<typename std::underlying_type<Enum>::type>::value,
                                  typename Integers::Unsigned, typename Integers::Signed>::type;

    Int i = 0;

    ~TzFlagsStorage() = default;
    TzFlagsStorage(const TzFlagsStorage &) = default;
    TzFlagsStorage(TzFlagsStorage &&) = default;
    TzFlagsStorage &operator=(const TzFlagsStorage &) = default;
    TzFlagsStorage &operator=(TzFlagsStorage &&) = default;

public:
    constexpr inline TzFlagsStorage() noexcept = default;
    constexpr inline explicit TzFlagsStorage(std::in_place_t, Int v)
        : i(v)
    {}
};

template<typename Enum, std::int32_t Size = sizeof(TzFlagsStorage<Enum>)>
struct TzFlagsStorageHelper : TzFlagsStorage<Enum>
{
    using TzFlagsStorage<Enum>::TzFlagsStorage;

protected:
    ~TzFlagsStorageHelper() = default;
    TzFlagsStorageHelper(const TzFlagsStorageHelper &) = default;
    TzFlagsStorageHelper(TzFlagsStorageHelper &&) = default;
    TzFlagsStorageHelper &operator=(const TzFlagsStorageHelper &) = default;
    TzFlagsStorageHelper &operator=(TzFlagsStorageHelper &&) = default;
};
template<typename Enum>
struct TzFlagsStorageHelper<Enum, sizeof(std::int32_t)> : TzFlagsStorage<Enum>
{
    using TzFlagsStorage<Enum>::TzFlagsStorage;

    constexpr inline TzFlagsStorageHelper(TzFlag flag) noexcept
        : TzFlagsStorage<Enum>(std::in_place, flag)
    {}

    constexpr inline explicit operator TzFlag() const noexcept { return TzFlag(this->i); }

protected:
    ~TzFlagsStorageHelper() = default;
    TzFlagsStorageHelper(const TzFlagsStorageHelper &) = default;
    TzFlagsStorageHelper(TzFlagsStorageHelper &&) = default;
    TzFlagsStorageHelper &operator=(const TzFlagsStorageHelper &) = default;
    TzFlagsStorageHelper &operator=(TzFlagsStorageHelper &&) = default;
};

} // namespace TzPrivate

template<typename Enum>
class TzFlags : public TzPrivate::TzFlagsStorageHelper<Enum>
{
    using Base = TzPrivate::TzFlagsStorageHelper<Enum>;

public:
    typedef Enum enum_type;
    using Int = typename Base::Int;
    using Base::Base;

    constexpr inline TzFlags() noexcept = default;

    constexpr inline TzFlags(Enum flags) noexcept
        : Base(std::in_place, Int(flags))
    {}

    constexpr inline TzFlags(TzFlag flag) noexcept
        requires(sizeof(Enum) == sizeof(std::int32_t));

    constexpr inline TzFlags(std::initializer_list<Enum> flags) noexcept
        : Base(std::in_place, initializer_list_helper(flags.begin(), flags.end()))
    {}

    constexpr static inline TzFlags fromInt(Int i) noexcept { return TzFlags(std::in_place, i); }
    constexpr inline Int toInt() const noexcept { return i; }

    constexpr inline TzFlags &operator&=(TzFlags mask) noexcept
    {
        i &= mask.i;
        return *this;
    }
    constexpr inline TzFlags &operator&=(Enum mask) noexcept
    {
        i &= Int(mask);
        return *this;
    }
    constexpr inline TzFlags &operator|=(TzFlags other) noexcept
    {
        i |= other.i;
        return *this;
    }
    constexpr inline TzFlags &operator|=(Enum other) noexcept
    {
        i |= Int(other);
        return *this;
    }
    constexpr inline TzFlags &operator^=(TzFlags other) noexcept
    {
        i ^= other.i;
        return *this;
    }
    constexpr inline TzFlags &operator^=(Enum other) noexcept
    {
        i ^= Int(other);
        return *this;
    }

    constexpr inline explicit operator Int() const noexcept { return i; }
    constexpr inline explicit operator bool() const noexcept { return i; }

    constexpr inline TzFlags operator|(TzFlags other) const noexcept
    {
        return TzFlags(std::in_place, i | other.i);
    }
    constexpr inline TzFlags operator|(Enum other) const noexcept
    {
        return TzFlags(std::in_place, i | Int(other));
    }
    constexpr inline TzFlags operator^(TzFlags other) const noexcept
    {
        return TzFlags(std::in_place, i ^ other.i);
    }
    constexpr inline TzFlags operator^(Enum other) const noexcept
    {
        return TzFlags(std::in_place, i ^ Int(other));
    }

    constexpr inline TzFlags operator&(TzFlags other) const noexcept
    {
        return TzFlags(std::in_place, i & other.i);
    }
    constexpr inline TzFlags operator&(Enum other) const noexcept
    {
        return TzFlags(std::in_place, i & Int(other));
    }
    constexpr inline TzFlags operator~() const noexcept { return TzFlags(std::in_place, ~i); }

    constexpr inline void operator+(TzFlags other) const noexcept = delete;
    constexpr inline void operator+(Enum other) const noexcept = delete;
    constexpr inline void operator+(std::int32_t other) const noexcept = delete;
    constexpr inline void operator-(TzFlags other) const noexcept = delete;
    constexpr inline void operator-(Enum other) const noexcept = delete;
    constexpr inline void operator-(std::int32_t other) const noexcept = delete;

    constexpr inline bool testFlag(Enum flag) const noexcept { return testFlags(flag); }
    constexpr inline bool testFlags(TzFlags flags) const noexcept
    {
        return flags.i ? ((i & flags.i) == flags.i) : i == Int(0);
    }
    constexpr inline bool testAnyFlag(Enum flag) const noexcept { return testAnyFlags(flag); }
    constexpr inline bool testAnyFlags(TzFlags flags) const noexcept
    {
        return (i & flags.i) != Int(0);
    }
    constexpr inline TzFlags &setFlag(Enum flag, bool on = true) noexcept
    {
        return on ? (*this |= flag) : (*this &= ~TzFlags(flag));
    }

    friend constexpr inline bool operator==(TzFlags lhs, TzFlags rhs) noexcept
    {
        return lhs.i == rhs.i;
    }
    friend constexpr inline bool operator!=(TzFlags lhs, TzFlags rhs) noexcept
    {
        return lhs.i != rhs.i;
    }
    friend constexpr inline bool operator==(TzFlags lhs, Enum rhs) noexcept
    {
        return lhs == TzFlags(rhs);
    }
    friend constexpr inline bool operator!=(TzFlags lhs, Enum rhs) noexcept
    {
        return lhs != TzFlags(rhs);
    }
    friend constexpr inline bool operator==(Enum lhs, TzFlags rhs) noexcept
    {
        return TzFlags(lhs) == rhs;
    }
    friend constexpr inline bool operator!=(Enum lhs, TzFlags rhs) noexcept
    {
        return TzFlags(lhs) != rhs;
    }

private:
    constexpr static inline Int initializer_list_helper(
        typename std::initializer_list<Enum>::const_iterator it,
        typename std::initializer_list<Enum>::const_iterator end) noexcept
    {
        return (it == end ? Int(0) : (Int(*it) | initializer_list_helper(it + 1, end)));
    }

    using Base::i;
};

#define TZ_DECLARE_FLAGS(Flags, Enum) typedef TzFlags<Enum> Flags;

#define TZ_DECLARE_TYPESAFE_OPERATORS_FOR_FLAGS_ENUM(Flags) \
    [[maybe_unused]] \
    constexpr inline Flags operator~(Flags::enum_type e) noexcept \
    { \
        return ~Flags(e); \
    } \
    [[maybe_unused]] \
    constexpr inline void operator|(Flags::enum_type f1, std::int32_t f2) noexcept \
        = delete;

#define TZ_DECLARE_OPERATORS_FOR_FLAGS(Flags) \
    [[maybe_unused]] \
    constexpr inline TzFlags<Flags::enum_type> operator|(Flags::enum_type f1, \
                                                         Flags::enum_type f2) noexcept \
    { \
        return TzFlags<Flags::enum_type>(f1) | f2; \
    } \
    [[maybe_unused]] \
    constexpr inline TzFlags<Flags::enum_type> operator|(Flags::enum_type f1, \
                                                         TzFlags<Flags::enum_type> f2) noexcept \
    { \
        return f2 | f1; \
    } \
    [[maybe_unused]] \
    constexpr inline TzFlags<Flags::enum_type> operator&(Flags::enum_type f1, \
                                                         Flags::enum_type f2) noexcept \
    { \
        return TzFlags<Flags::enum_type>(f1) & f2; \
    } \
    [[maybe_unused]] \
    constexpr inline TzFlags<Flags::enum_type> operator&(Flags::enum_type f1, \
                                                         TzFlags<Flags::enum_type> f2) noexcept \
    { \
        return f2 & f1; \
    } \
    [[maybe_unused]] \
    constexpr inline TzFlags<Flags::enum_type> operator^(Flags::enum_type f1, \
                                                         Flags::enum_type f2) noexcept \
    { \
        return TzFlags<Flags::enum_type>(f1) ^ f2; \
    } \
    [[maybe_unused]] \
    constexpr inline TzFlags<Flags::enum_type> operator^(Flags::enum_type f1, \
                                                         TzFlags<Flags::enum_type> f2) noexcept \
    { \
        return f2 ^ f1; \
    } \
    constexpr inline void operator+(Flags::enum_type f1, Flags::enum_type f2) noexcept = delete; \
    constexpr inline void operator+(Flags::enum_type f1, TzFlags<Flags::enum_type> f2) noexcept \
        = delete; \
    constexpr inline void operator+(std::int32_t f1, TzFlags<Flags::enum_type> f2) noexcept \
        = delete; \
    constexpr inline void operator-(Flags::enum_type f1, Flags::enum_type f2) noexcept = delete; \
    constexpr inline void operator-(Flags::enum_type f1, TzFlags<Flags::enum_type> f2) noexcept \
        = delete; \
    constexpr inline void operator-(std::int32_t f1, TzFlags<Flags::enum_type> f2) noexcept \
        = delete; \
    constexpr inline void operator+(std::int32_t f1, Flags::enum_type f2) noexcept = delete; \
    constexpr inline void operator+(Flags::enum_type f1, std::int32_t f2) noexcept = delete; \
    constexpr inline void operator-(std::int32_t f1, Flags::enum_type f2) noexcept = delete; \
    constexpr inline void operator-(Flags::enum_type f1, std::int32_t f2) noexcept = delete; \
    TZ_DECLARE_TYPESAFE_OPERATORS_FOR_FLAGS_ENUM(Flags)

#define TZ_DECLARE_MIXED_ENUM_OPERATOR(op, Ret, LHS, RHS) \
    [[maybe_unused]] \
    constexpr inline Ret operator op(LHS lhs, RHS rhs) noexcept \
    { \
        return static_cast<Ret>(qToUnderlying(lhs) op qToUnderlying(rhs)); \
    } \
    /* end */

#define TZ_DECLARE_MIXED_ENUM_OPERATORS(Ret, Flags, Enum) \
    TZ_DECLARE_MIXED_ENUM_OPERATOR(|, Ret, Flags, Enum) \
    TZ_DECLARE_MIXED_ENUM_OPERATOR(&, Ret, Flags, Enum) \
    TZ_DECLARE_MIXED_ENUM_OPERATOR(^, Ret, Flags, Enum) \
    /* end */

#define TZ_DECLARE_MIXED_ENUM_OPERATORS_SYMMETRIC(Ret, Flags, Enum) \
    TZ_DECLARE_MIXED_ENUM_OPERATORS(Ret, Flags, Enum) \
    TZ_DECLARE_MIXED_ENUM_OPERATORS(Ret, Enum, Flags) \
    /* end */

#endif // TZFLAGS_HPP
