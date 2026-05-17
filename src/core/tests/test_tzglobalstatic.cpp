#include <loom/tzglobalstatic.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <string>
#include <thread>
#include <utility>
#include <vector>

TZ_GLOBAL_STATIC(int, gInteger)
TZ_GLOBAL_STATIC(std::string, gString)
TZ_GLOBAL_STATIC_WITH_ARGS(std::string, gStringHello, ("hello"))

struct Point {
    int x, y;
    Point(int x_, int y_) : x(x_), y(y_) {}
};
TZ_GLOBAL_STATIC_WITH_ARGS(Point, gPoint, (10, 20))

struct Tracked {
    static std::atomic<int> live;
    int value;
    explicit Tracked(int v = 99) : value(v) { ++live; }
    ~Tracked() { --live; }
};
std::atomic<int> Tracked::live{0};

TZ_GLOBAL_STATIC(Tracked, gTracked)

struct Concurrent {
    static std::atomic<int> ctorCalls;
    Concurrent() { ++ctorCalls; }
};
std::atomic<int> Concurrent::ctorCalls{0};

TZ_GLOBAL_STATIC(Concurrent, gConcurrent)

TEST(TzGlobalStatic, DefaultInitialisedInt)
{
    // operator* must return the value-initialised int (0)
    EXPECT_EQ(*gInteger, 0);
}

TEST(TzGlobalStatic, DefaultInitialisedString)
{
    EXPECT_EQ(*gString, std::string{});
}

TEST(TzGlobalStatic, WithArgsString)
{
    EXPECT_EQ(*gStringHello, "hello");
}

TEST(TzGlobalStatic, WithArgsStruct)
{
    EXPECT_EQ(gPoint->x, 10);
    EXPECT_EQ(gPoint->y, 20);
}

TEST(TzGlobalStatic, ExistsAfterFirstAccess)
{
    (void)*gInteger; // force initialisation
    EXPECT_TRUE(gInteger.exists());
    EXPECT_FALSE(gInteger.isDestroyed());
}

TEST(TzGlobalStatic, NotDestroyedDuringLifetime)
{
    (void)*gTracked;
    EXPECT_FALSE(gTracked.isDestroyed());
}

TEST(TzGlobalStatic, PointerIdentityAcrossAccesses)
{
    int *p1 = gInteger;
    int *p2 = gInteger;
    EXPECT_NE(p1, nullptr);
    EXPECT_EQ(p1, p2);
}

TEST(TzGlobalStatic, ConstructedExactlyOnce)
{
    // Force initialisation and verify exactly one live instance
    (void)*gTracked;
    EXPECT_EQ(Tracked::live.load(), 1);

    // A second access must not create another instance
    (void)*gTracked;
    EXPECT_EQ(Tracked::live.load(), 1);
}

TEST(TzGlobalStatic, ValueAfterConstruction)
{
    EXPECT_EQ(gTracked->value, 99);
}

TEST(TzGlobalStatic, ImplicitConversionToPointer)
{
    int *p = gInteger;          // operator Type*()
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(*p, 0);
}

TEST(TzGlobalStatic, FunctionCallOperator)
{
    int *p = gInteger();        // operator()()
    EXPECT_NE(p, nullptr);
    EXPECT_EQ(p, static_cast<int *>(gInteger));
}

TEST(TzGlobalStatic, ArrowOperator)
{
    // operator->() on a struct type
    EXPECT_EQ(gPoint->x, 10);
    EXPECT_EQ(gPoint->y, 20);
}

TEST(TzGlobalStatic, DereferenceOperator)
{
    Point &ref = *gPoint;   // operator*()
    EXPECT_EQ(ref.x, 10);
    EXPECT_EQ(ref.y, 20);
}

TEST(TzGlobalStatic, WriteThrough)
{
    // The global static is mutable; writes should persist
    int *p = gInteger;
    int old = *p;
    *p = 42;
    EXPECT_EQ(*gInteger, 42);
    *p = old; // restore for other tests
}

TEST(TzGlobalStatic, ConcurrentAccessReturnsSamePointer)
{
    constexpr int kThreads = 16;
    std::vector<Concurrent *> results(kThreads, nullptr);
    std::vector<std::thread> threads;
    threads.reserve(kThreads);

    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&results, i] {
            results[i] = gConcurrent; // operator Type*()
        });
    }
    for (auto &t : threads)
        t.join();

    Concurrent *expected = results[0];
    EXPECT_NE(expected, nullptr);
    for (int i = 1; i < kThreads; ++i)
        EXPECT_EQ(results[i], expected);
}

TEST(TzGlobalStatic, ConcurrentConstructorCalledOnce)
{
    // gConcurrent may have been initialised by a previous test; in any case
    // ctorCalls must be exactly 1 by the time all threads have finished.
    (void)*gConcurrent;
    EXPECT_EQ(Concurrent::ctorCalls.load(), 1);
}
