#include <loom/tzoverload.hpp>
#include <gtest/gtest.h>
#include <cmath>
#include <string>

struct Widget {
    int value_ = 0;
    std::string name_;

    void setValue(int v) { value_ = v; }
    void setValue(double v) { value_ = static_cast<int>(v * 10); }

    int value() const { return value_; }
    std::string value(const std::string &prefix) const { return prefix + name_; }

    void reset() noexcept { value_ = 0; name_.clear(); }

    static int staticAdd(int a, int b) { return a + b; }
    static double staticAdd(double a, double b) { return a + b; }
};


int freeAdd(int a, int b) { return a + b; }
double freeAdd(double a, double b) { return a + b; }

TEST(TzOverload, StructForm_NonConstMemberFunction)
{
    auto pmf = TzOverload<int>::of(&Widget::setValue);

    Widget w;
    (w.*pmf)(7);
    EXPECT_EQ(w.value_, 7);
}

TEST(TzOverload, StructForm_NonConstMemberFunctionDoubleOverload)
{
    auto pmf = TzOverload<double>::of(&Widget::setValue);

    Widget w;
    (w.*pmf)(1.5); 
    EXPECT_EQ(w.value_, 15);
}

TEST(TzOverload, StructForm_FreeFunction)
{
    auto pf = TzOverload<int, int>::of(&freeAdd);

    EXPECT_EQ(pf(3, 4), 7);
}

TEST(TzOverload, StructForm_StaticMemberFunction)
{
    auto pf = TzOverload<double, double>::of(&Widget::staticAdd);

    EXPECT_DOUBLE_EQ(pf(1.5, 2.5), 4.0);
}

TEST(TzConstOverload, StructForm_ConstMemberFunction)
{
    auto pmf = TzConstOverload<>::of(&Widget::value);

    Widget w;
    w.value_ = 42;
    EXPECT_EQ((w.*pmf)(), 42);
}

TEST(TzConstOverload, StructForm_ConstMemberFunctionWithArg)
{
    auto pmf = TzConstOverload<const std::string &>::of(&Widget::value);

    Widget w;
    w.name_ = "loom";
    EXPECT_EQ((w.*pmf)("lib-"), "lib-loom");
}

TEST(TzOverload, HelperFn_NonConstMember)
{
    auto pmf = tzOverload<int>(&Widget::setValue);

    Widget w;
    (w.*pmf)(99);
    EXPECT_EQ(w.value_, 99);
}

TEST(TzOverload, HelperFn_FreeFunction)
{
    auto pf = tzOverload<int, int>(&freeAdd);

    EXPECT_EQ(pf(10, 20), 30);
}

TEST(TzConstOverload, HelperFn_ConstMember)
{
    struct Counter {
        int count = 0;
        int get() const { return count; }
    };

    auto pmf = tzConstOverload<>(&Counter::get);
    Counter c;
    c.count = 5;
    EXPECT_EQ((c.*pmf)(), 5);
}

TEST(TzNoexceptOverload, StructForm_NoexceptMember)
{
    auto pmf = TzNoexceptOverload<>::of(&Widget::reset);

    Widget w;
    w.value_ = 123;
    w.name_ = "x";
    (w.*pmf)();
    EXPECT_EQ(w.value_, 0);
    EXPECT_TRUE(w.name_.empty());
}

TEST(TzNoexceptOverload, HelperFn_NoexceptMember)
{
    auto pmf = tzNoexceptOverload<>(&Widget::reset);

    Widget w;
    w.value_ = 7;
    (w.*pmf)();
    EXPECT_EQ(w.value_, 0);
}

TEST(TzOverload, TypeTraits_NonConstMember)
{
    using PMF = void (Widget::*)(int);
    auto pmf = TzOverload<int>::of(&Widget::setValue);
    static_assert(std::is_same_v<decltype(pmf), PMF>);
    SUCCEED();
}

TEST(TzConstOverload, TypeTraits_ConstMember)
{
    using PMF = int (Widget::*)() const;
    auto pmf = TzConstOverload<>::of(&Widget::value);
    static_assert(std::is_same_v<decltype(pmf), PMF>);
    SUCCEED();
}

TEST(TzNoexceptConstOverload, TypeTraits_NoexceptConst)
{
    struct S {
        int get() const noexcept { return 0; }
        int get(int x) const noexcept { return x; }
    };

    using PMF = int (S::*)() const noexcept;
    auto pmf = TzNoexceptConstOverload<>::of(&S::get);
    static_assert(std::is_same_v<decltype(pmf), PMF>);

    S s;
    EXPECT_EQ((s.*pmf)(), 0);
}
