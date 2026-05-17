#include <loom/tzassert.hpp>
#include <cstdio>
#include <cstdlib>
#include <new>

void tzAssert(const char *assertion, const char *file, int line) noexcept
{
    std::fprintf(stderr, "ASSERT: \"%s\" in file %s, line %d\n", assertion, file, line);
    std::abort();
}

void tzAssertX(const char *where, const char *what, const char *file, int line) noexcept
{
    std::fprintf(stderr, "ASSERT failure in %s: \"%s\", file %s, line %d\n",
                 where, what, file, line);
    std::abort();
}

void tzCheckPointer(const char *file, int line) noexcept
{
    std::fprintf(stderr, "FATAL: null pointer dereference in file %s, line %d\n", file, line);
    std::abort();
}

void tzBadAlloc()
{
    throw std::bad_alloc{};
}
