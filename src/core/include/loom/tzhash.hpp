#ifndef TZHASH_HPP
#define TZHASH_HPP

#include <cstdint>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace TzHashPrivate {

constexpr size_t hash(size_t key, size_t seed) noexcept
{
    key ^= seed;
    if constexpr (sizeof(size_t) == 4) {
        key ^= key >> 16;
        key *= UINT32_C(0x45d9f3b);
        key ^= key >> 16;
        key *= UINT32_C(0x45d9f3b);
        key ^= key >> 16;
        return key;
    } else {
        std::uint64_t key64 = key;
        key64 ^= key64 >> 32;
        key64 *= UINT64_C(0xd6e8feb86659fd93);
        key64 ^= key64 >> 32;
        key64 *= UINT64_C(0xd6e8feb86659fd93);
        key64 ^= key64 >> 32;
        return size_t(key64);
    }
}

template <typename T1, typename T2>
static constexpr bool noexceptPairHash();

} // namespace TzHashPrivate

template <typename T1, typename T2> 
inline size_t tzHash(const std::pair<T1, T2> &key, size_t seed = 0)
    noexcept(TzHashPrivate::noexceptPairHash<T1, T2>());

constexpr inline size_t tzHash(char key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(unsigned char key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(signed char key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(unsigned short key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(short key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(unsigned int key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(int key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(unsigned long key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(long key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }
constexpr inline size_t tzHash(std::uint64_t key, size_t seed = 0) noexcept
{
    if constexpr (sizeof(std::uint64_t) > sizeof(size_t))
        key ^= (key >> 32);
    return TzHashPrivate::hash(size_t(key), seed);
}
constexpr inline size_t tzHash(std::int64_t key, size_t seed = 0) noexcept
{
    if constexpr (sizeof(std::int64_t) > sizeof(size_t)) {
        std::uint32_t high = std::uint32_t(std::uint64_t(key) >> 32);
        std::uint32_t low = std::uint32_t(std::uint64_t(key));
        std::uint32_t signmask = std::int32_t(high) >> 31;  // all zeroes or all ones
        low ^= signmask ^ high;
        return tzHash(low, seed);
    }
    return tzHash(std::uint64_t(key), seed);
}
inline size_t tzHash(float key, size_t seed = 0) noexcept
{
    // ensure -0 gets mapped to 0
    key += 0.0f;
    unsigned int k;
    memcpy(&k, &key, sizeof(float));
    return TzHashPrivate::hash(k, seed);
}
size_t tzHash(double key, size_t seed = 0) noexcept;
size_t tzHash(long double key, size_t seed = 0) noexcept;

constexpr inline size_t tzHash(wchar_t key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }

constexpr inline size_t tzHash(char16_t key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }

constexpr inline size_t tzHash(char32_t key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }

constexpr inline size_t tzHash(char8_t key, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(size_t(key), seed); }

template <class T>
inline size_t tzHash(const T *key, size_t seed = 0) noexcept
{ return tzHash(reinterpret_cast<std::uintptr_t>(key), seed); }

constexpr inline size_t tzHash(std::nullptr_t, size_t seed = 0) noexcept
{ return seed; }

template <class Enum, std::enable_if_t<std::is_enum_v<Enum>, bool> = true>
constexpr inline size_t tzHash(Enum e, size_t seed = 0) noexcept
{ return TzHashPrivate::hash(tzToUnderlying(e), seed); }

size_t tzHash(std::string_view key, size_t seed = 0) noexcept;

inline size_t tzHash(const std::string &key, size_t seed = 0) noexcept
{ return tzHash(std::string_view{key}, seed); }

template <typename Enum>
constexpr inline size_t tzHash(TzFlags<Enum> flags, size_t seed = 0) noexcept
{ return tzHash(flags.toInt(), seed); }

namespace TzPrivate {

struct QHashCombine
{
    template <typename T>
    constexpr size_t operator()(size_t seed, const T &t) const noexcept(noexcept(tzHash(t)))
    { return seed ^ (tzHash(t) + 0x9e3779b9 + (seed << 6) + (seed >> 2)) ; }
};

} // namespace TzPrivate

template <typename... T>
constexpr size_t tzHashMulti(size_t seed, const T &... args)
{
    TzPrivate::QHashCombine hash;
    return ((seed = hash(seed, args)), ...), seed;
}

#endif // TZHASH_HPP
