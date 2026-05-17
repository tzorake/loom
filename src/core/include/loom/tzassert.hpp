#ifndef TZASSERT_HPP
#define TZASSERT_HPP

void tzAssert(const char *assertion, const char *file, int line) noexcept;

#if !defined(TZ_ASSERT)
#  if defined(TZ_NO_DEBUG) && !defined(TZ_FORCE_ASSERTS)
#    define TZ_ASSERT(cond) static_cast<void>(false && (cond))
#  else
#    define TZ_ASSERT(cond) ((cond) ? static_cast<void>(0) : tzAssert(#cond, __FILE__, __LINE__))
#  endif
#endif

void tzAssertX(const char *where, const char *what, const char *file, int line) noexcept;
inline bool tzNoAssertX(bool, const char *, const char *) noexcept { return false; }

#if !defined(TZ_ASSERT_X)
#  if defined(TZ_NO_DEBUG) && !defined(TZ_FORCE_ASSERTS)
#    define TZ_ASSERT_X(cond, where, what) static_cast<void>(false && tzNoAssertX(bool(cond), where, what))
#  else
#    define TZ_ASSERT_X(cond, where, what) ((cond) ? static_cast<void>(0) : tzAssertX(where, what, __FILE__, __LINE__))
#  endif
#endif

void tzCheckPointer(const char *, int) noexcept;
void tzBadAlloc();

#ifdef TZ_NO_EXCEPTIONS
#  if defined(TZ_NO_DEBUG) && !defined(TZ_FORCE_ASSERTS)
#    define TZ_CHECK_PTR(p) tzNoop()
#  else
#    define TZ_CHECK_PTR(p) do {if (!(p)) tzCheckPointer(__FILE__,__LINE__);} while (false)
#  endif
#else
#  define TZ_CHECK_PTR(p) do { if (!(p)) tzBadAlloc(); } while (false)
#endif

template <typename T>
inline T *tzCheckPtr(T *p) { TZ_CHECK_PTR(p); return p; }

#define TZ_UNREACHABLE() \
    do {\
        TZ_ASSERT_X(false, "TZ_UNREACHABLE()", "TZ_UNREACHABLE was reached");\
        TZ_UNREACHABLE_IMPL();\
    } while (false)

#ifndef TZ_UNREACHABLE_RETURN
#  define TZ_UNREACHABLE_RETURN(...) do { TZ_UNREACHABLE(); return __VA_ARGS__; } while (0)
#endif

#endif // TZASSERT_HPP
