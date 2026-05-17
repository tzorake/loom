#ifndef TZGLOBALSTATIC_HPP
#define TZGLOBALSTATIC_HPP

#include <loom/tzassert.hpp>
#include <loom/tzclasshelpermacros.hpp>
#include <atomic>
#include <cstdint>
#include <type_traits>

namespace TzGlobalStaticPrivate {

enum GuardValues {
    Destroyed = -2,
    Initialized = -1,
    Uninitialized =  0,
    Initializing =  1
};

template <typename TzGS>
union Holder
{
    using Type = typename TzGS::TzGlobalStatic_Type;
    using PlainType = std::remove_cv_t<Type>;

    static constexpr bool ConstructionIsNoexcept = noexcept(TzGS::innerFunction(nullptr));
    static inline std::atomic<int8_t> guard = { TzGlobalStaticPrivate::Uninitialized };

    PlainType storage;

    Holder() noexcept(ConstructionIsNoexcept)
    {
        TzGS::innerFunction(pointer());
        guard.store(TzGlobalStaticPrivate::Initialized, std::memory_order_relaxed);
    }

    ~Holder()
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        pointer()->~PlainType();
        guard.store(TzGlobalStaticPrivate::Destroyed, std::memory_order_release);
    }

    PlainType *pointer() noexcept
    {
        return &storage;
    }

    TZ_DISABLE_COPY_MOVE(Holder)
};

} // namespace TzGlobalStaticPrivate

template <typename Holder>
struct TzGlobalStatic
{
    using Type = typename Holder::Type;

    bool isDestroyed() const noexcept { return guardValue() <= TzGlobalStaticPrivate::Destroyed; }
    bool exists() const noexcept { return guardValue() == TzGlobalStaticPrivate::Initialized; }

    operator Type *()
    {
        if (isDestroyed())
            return nullptr;
        return instance();
    }
    Type *operator()()
    {
        if (isDestroyed())
            return nullptr;
        return instance();
    }
    Type *operator->()
    {
        TZ_ASSERT_X(!isDestroyed(), __func__,
                    "The global static was used after being destroyed");
        return instance();
    }
    Type &operator*()
    {
        TZ_ASSERT_X(!isDestroyed(), __func__,
                    "The global static was used after being destroyed");
        return *instance();
    }

protected:
    static Type *instance() noexcept(Holder::ConstructionIsNoexcept)
    {
        static Holder holder;
        return holder.pointer();
    }
    static TzGlobalStaticPrivate::GuardValues guardValue() noexcept
    {
        return TzGlobalStaticPrivate::GuardValues(Holder::guard.load(std::memory_order_acquire));
    }
};

#define TZ_GLOBAL_STATIC_WITH_ARGS(type, name, args)                                    \
    namespace {                                                                         \
    struct TzGlobalStatic_ ## name {                                                    \
        using TzGlobalStatic_Type = type;                                               \
        static void innerFunction(void *pointer)                                        \
            noexcept(noexcept(std::remove_cv_t<TzGlobalStatic_Type> args))              \
        {                                                                               \
            new (pointer) TzGlobalStatic_Type args;                                     \
        }                                                                               \
    };                                                                                  \
    } /* anonymous namespace */                                                         \
    static TzGlobalStatic<TzGlobalStaticPrivate::Holder<TzGlobalStatic_ ## name>> name; \
    /**/

#define TZ_GLOBAL_STATIC(type, name, ...) \
    TZ_GLOBAL_STATIC_WITH_ARGS(type, name, (__VA_ARGS__))

#endif // TZGLOBALSTATIC_HPP
