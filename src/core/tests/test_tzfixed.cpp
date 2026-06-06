#include <gtest/gtest.h>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>

#include "../src/tzbigint.h"
#include "../src/tzfixed.h"

#include <loom/tzfixed.hpp>

class FixedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        pi = fixed_from_double(3.141592653589793, 16);
        two = fixed_from_double(2.0, 16);
        zero = fixed_from_double(0.0, 16);
        negative = fixed_from_double(-5.5, 16);
    }

    void TearDown() override
    {
        fixed_destroy(pi);
        fixed_destroy(two);
        fixed_destroy(zero);
        fixed_destroy(negative);
    }

    fixed_t* pi;
    fixed_t* two;
    fixed_t* zero;
    fixed_t* negative;
};

TEST_F(FixedTest, CreateFromDouble)
{
    fixed_t* num = fixed_from_double(3.14, 16);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(3.14, fixed_to_double(num), 0.001);
    fixed_destroy(num);
}

TEST_F(FixedTest, CreateFromString)
{
    fixed_t* num = fixed_from_string("3.14159", 16);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(3.14159, fixed_to_double(num), 0.001);
    fixed_destroy(num);
}

TEST_F(FixedTest, CreateFromNegativeString)
{
    fixed_t* num = fixed_from_string("-2.71828", 16);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(-2.71828, fixed_to_double(num), 0.001);
    fixed_destroy(num);
}

TEST_F(FixedTest, CreateFromStringNoDecimal)
{
    fixed_t* num = fixed_from_string("42", 16);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(42.0, fixed_to_double(num), 0.001);
    fixed_destroy(num);
}

TEST_F(FixedTest, CreateFromEmptyFraction)
{
    fixed_t* num = fixed_from_string("123.", 16);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(123.0, fixed_to_double(num), 0.001);
    fixed_destroy(num);
}

TEST_F(FixedTest, CreateFromLeadingDecimal)
{
    fixed_t* num = fixed_from_string(".75", 16);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(0.75, fixed_to_double(num), 0.001);
    fixed_destroy(num);
}

TEST_F(FixedTest, Clone)
{
    fixed_t* clone = fixed_clone(pi);
    ASSERT_NE(clone, nullptr);
    EXPECT_EQ(0, fixed_cmp(pi, clone));
    fixed_destroy(clone);
}

TEST_F(FixedTest, GetFracBits)
{
    EXPECT_EQ(16, fixed_get_frac_bits(pi));
    EXPECT_EQ(16, fixed_get_frac_bits(two));
}

TEST_F(FixedTest, GetNumerator)
{
    bigint_t* num = fixed_get_numerator(pi);
    ASSERT_NE(num, nullptr);
    EXPECT_FALSE(bigint_is_zero(num));
}

TEST_F(FixedTest, ToDouble)
{
    EXPECT_NEAR(3.141592653589793, fixed_to_double(pi), 0.001);
    EXPECT_NEAR(2.0, fixed_to_double(two), 0.001);
    EXPECT_NEAR(0.0, fixed_to_double(zero), 0.001);
    EXPECT_NEAR(-5.5, fixed_to_double(negative), 0.001);
}

TEST_F(FixedTest, ToString)
{
    char* str = fixed_to_string(pi);
    EXPECT_NE(str, nullptr);
    EXPECT_GT(strlen(str), 0u);
    free(str);
}

TEST_F(FixedTest, RescaleHigher)
{
    fixed_t* rescaled = fixed_rescale(pi, 32);
    ASSERT_NE(rescaled, nullptr);
    EXPECT_EQ(32, fixed_get_frac_bits(rescaled));
    EXPECT_NEAR(3.141592653589793, fixed_to_double(rescaled), 0.001);
    fixed_destroy(rescaled);
}

TEST_F(FixedTest, RescaleLower)
{
    fixed_t* rescaled = fixed_rescale(pi, 8);
    ASSERT_NE(rescaled, nullptr);
    EXPECT_EQ(8, fixed_get_frac_bits(rescaled));
    EXPECT_NEAR(3.140625, fixed_to_double(rescaled), 0.001);  // lower precision
    fixed_destroy(rescaled);
}

TEST_F(FixedTest, RescaleSame)
{
    fixed_t* rescaled = fixed_rescale(pi, 16);
    ASSERT_NE(rescaled, nullptr);
    EXPECT_EQ(16, fixed_get_frac_bits(rescaled));
    EXPECT_NEAR(3.141592653589793, fixed_to_double(rescaled), 0.001);
    fixed_destroy(rescaled);
}

TEST_F(FixedTest, Addition)
{
    fixed_t* result = fixed_add(pi, two);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(5.141592653589793, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, AdditionInPlace)
{
    fixed_t* a = fixed_clone(pi);
    ASSERT_EQ(0, fixed_add_assign(a, two));
    EXPECT_NEAR(5.141592653589793, fixed_to_double(a), 0.001);
    fixed_destroy(a);
}

TEST_F(FixedTest, AdditionWithZero)
{
    fixed_t* result = fixed_add(pi, zero);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(3.141592653589793, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, Subtraction)
{
    fixed_t* result = fixed_sub(pi, two);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(1.141592653589793, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, SubtractionInPlace)
{
    fixed_t* a = fixed_clone(pi);
    ASSERT_EQ(0, fixed_sub_assign(a, two));
    EXPECT_NEAR(1.141592653589793, fixed_to_double(a), 0.001);
    fixed_destroy(a);
}

TEST_F(FixedTest, SubtractionNegative)
{
    fixed_t* result = fixed_sub(two, pi);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(-1.141592653589793, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, Multiplication)
{
    fixed_t* result = fixed_mul(pi, two);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(6.283185307179586, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, MultiplicationInPlace)
{
    fixed_t* a = fixed_clone(pi);
    ASSERT_EQ(0, fixed_mul_assign(a, two));
    EXPECT_NEAR(6.283185307179586, fixed_to_double(a), 0.001);
    fixed_destroy(a);
}

TEST_F(FixedTest, MultiplicationByZero)
{
    fixed_t* result = fixed_mul(pi, zero);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(0.0, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, Division)
{
    fixed_t* result = fixed_div(pi, two);
    ASSERT_NE(result, nullptr);
    EXPECT_NEAR(1.5707963267948966, fixed_to_double(result), 0.001);
    fixed_destroy(result);
}

TEST_F(FixedTest, DivisionInPlace)
{
    fixed_t* a = fixed_clone(pi);
    ASSERT_EQ(0, fixed_div_assign(a, two));
    EXPECT_NEAR(1.5707963267948966, fixed_to_double(a), 0.001);
    fixed_destroy(a);
}

TEST_F(FixedTest, DivisionByZero)
{
    fixed_t* result = fixed_div(pi, zero);
    EXPECT_EQ(result, nullptr);
}

TEST_F(FixedTest, CompareEqual)
{
    fixed_t* a = fixed_clone(pi);
    EXPECT_EQ(0, fixed_cmp(pi, a));
    fixed_destroy(a);
}

TEST_F(FixedTest, CompareLess)
{
    fixed_t* small = fixed_from_double(1.5, 16);
    fixed_t* large = fixed_from_double(3.0, 16);

    EXPECT_LT(fixed_cmp(small, large), 0);
    EXPECT_GT(fixed_cmp(large, small), 0);

    fixed_destroy(small);
    fixed_destroy(large);
}

TEST_F(FixedTest, CompareDifferentFracBits)
{
    fixed_t* a = fixed_from_double(3.14159, 32);
    fixed_t* b = fixed_from_double(3.14159, 16);

    EXPECT_EQ(0, fixed_cmp(a, b));

    fixed_destroy(a);
    fixed_destroy(b);
}

TEST(FixedEdgeTest, VeryHighPrecision)
{
    fixed_t* num = fixed_from_double(0.123456789, 64);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(0.123456789, fixed_to_double(num), 1e-9);
    fixed_destroy(num);
}

TEST(FixedEdgeTest, VeryLargeNumber)
{
    fixed_t* num = fixed_from_string("12345678901234567890.123456789", 32);
    ASSERT_NE(num, nullptr);
    double result = fixed_to_double(num);
    EXPECT_GT(result, 1.234e19);
    fixed_destroy(num);
}

TEST(FixedEdgeTest, VerySmallNumber)
{
    fixed_t* num = fixed_from_double(1e-10, 64);
    ASSERT_NE(num, nullptr);
    EXPECT_NEAR(1e-10, fixed_to_double(num), 1e-15);
    fixed_destroy(num);
}

TEST(TzFixed, ConstructFromFracBitsIsZero)
{
    TzFixed a(16);
    EXPECT_EQ(16, a.fracBits());
    EXPECT_NEAR(0.0, a.toDouble(), 1e-9);
}

TEST(TzFixed, ConstructFromDouble)
{
    TzFixed a(3.14, 16);
    EXPECT_EQ(16, a.fracBits());
    EXPECT_NEAR(3.14, a.toDouble(), 0.001);
}

TEST(TzFixed, ConstructFromDoubleNegative)
{
    TzFixed a(-2.71828, 16);
    EXPECT_NEAR(-2.71828, a.toDouble(), 0.001);
}

TEST(TzFixed, ConstructFromCString)
{
    TzFixed a("3.14159", 16);
    EXPECT_NEAR(3.14159, a.toDouble(), 0.001);
}

TEST(TzFixed, ConstructFromStdString)
{
    TzFixed a(std::string("2.71828"), 16);
    EXPECT_NEAR(2.71828, a.toDouble(), 0.001);
}

TEST(TzFixed, ConstructFromNegativeString)
{
    TzFixed a("-1.5", 16);
    EXPECT_NEAR(-1.5, a.toDouble(), 0.001);
}

TEST(TzFixed, CopyConstructor)
{
    TzFixed a(3.14, 16);
    TzFixed b(a);
    EXPECT_NEAR(a.toDouble(), b.toDouble(), 1e-9);
    EXPECT_EQ(a.fracBits(), b.fracBits());
    b += TzFixed(1.0, 16);
    EXPECT_NEAR(3.14, a.toDouble(), 0.001);
}

TEST(TzFixed, MoveConstructor)
{
    TzFixed a(2.5, 16);
    TzFixed b(std::move(a));
    EXPECT_NEAR(2.5, b.toDouble(), 0.001);
}

TEST(TzFixed, CopyAssignment)
{
    TzFixed a(1.5, 16);
    TzFixed b(16);
    b = a;
    EXPECT_NEAR(1.5, b.toDouble(), 0.001);
    b += TzFixed(1.0, 16);
    EXPECT_NEAR(1.5, a.toDouble(), 0.001);
}

TEST(TzFixed, CopyAssignmentSelf)
{
    TzFixed a(3.14, 16);
    a = a;
    EXPECT_NEAR(3.14, a.toDouble(), 0.001);
}

TEST(TzFixed, MoveAssignment)
{
    TzFixed a(7.5, 16);
    TzFixed b(16);
    b = std::move(a);
    EXPECT_NEAR(7.5, b.toDouble(), 0.001);
}

TEST(TzFixed, FracBits)
{
    EXPECT_EQ(8,  TzFixed(0.0, 8).fracBits());
    EXPECT_EQ(32, TzFixed(0.0, 32).fracBits());
}

TEST(TzFixed, Numerator)
{
    TzFixed a(4.0, 8); // numerator = 4 * 2^8 = 1024
    EXPECT_DOUBLE_EQ(1024.0, a.numerator().toDouble());
}

TEST(TzFixed, NumeratorIsAClone)
{
    TzFixed a(2.0, 8);
    TzBigInt num = a.numerator();
    num += TzBigInt(1LL);
    EXPECT_NEAR(2.0, a.toDouble(), 0.001);
}

TEST(TzFixed, ToDouble)
{
    EXPECT_NEAR(1.25, TzFixed(1.25, 16).toDouble(), 1e-6);
}

TEST(TzFixed, ToStringNotEmpty)
{
    EXPECT_FALSE(TzFixed(3.5, 16).toString().empty());
}

TEST(TzFixed, RescaleHigher)
{
    TzFixed b = TzFixed(3.14159, 16).rescale(32);
    EXPECT_EQ(32, b.fracBits());
    EXPECT_NEAR(3.14159, b.toDouble(), 0.001);
}

TEST(TzFixed, RescaleLower)
{
    TzFixed b = TzFixed(3.14159, 16).rescale(8);
    EXPECT_EQ(8, b.fracBits());
    EXPECT_NEAR(3.140625, b.toDouble(), 0.01);
}

TEST(TzFixed, RescaleSame)
{
    TzFixed b = TzFixed(3.14159, 16).rescale(16);
    EXPECT_EQ(16, b.fracBits());
    EXPECT_NEAR(3.14159, b.toDouble(), 0.001);
}

TEST(TzFixed, RescaleDoesNotMutateOriginal)
{
    TzFixed a(1.5, 16);
    (void)a.rescale(32);
    EXPECT_EQ(16, a.fracBits());
    EXPECT_NEAR(1.5, a.toDouble(), 0.001);
}

TEST(TzFixed, AddAssign)
{
    TzFixed a(1.5, 16), b(2.5, 16);
    a += b;
    EXPECT_NEAR(4.0, a.toDouble(), 0.001);
}

TEST(TzFixed, AddAssignMismatchThrows)
{
    TzFixed a(1.0, 16), b(1.0, 8);
    EXPECT_THROW(a += b, std::invalid_argument);
}

TEST(TzFixed, SubAssign)
{
    TzFixed a(5.0, 16), b(2.0, 16);
    a -= b;
    EXPECT_NEAR(3.0, a.toDouble(), 0.001);
}

TEST(TzFixed, SubAssignMismatchThrows)
{
    TzFixed a(1.0, 16), b(1.0, 8);
    EXPECT_THROW(a -= b, std::invalid_argument);
}

TEST(TzFixed, SubAssignNegativeResult)
{
    TzFixed a(1.0, 16), b(3.0, 16);
    a -= b;
    EXPECT_NEAR(-2.0, a.toDouble(), 0.001);
}

TEST(TzFixed, MulAssign)
{
    TzFixed a(3.0, 16), b(2.5, 16);
    a *= b;
    EXPECT_NEAR(7.5, a.toDouble(), 0.01);
}

TEST(TzFixed, DivAssign)
{
    TzFixed a(7.5, 16), b(2.5, 16);
    a /= b;
    EXPECT_NEAR(3.0, a.toDouble(), 0.01);
}

TEST(TzFixed, OperatorAdd)
{
    TzFixed a(1.0, 16), b(2.0, 16);
    TzFixed c = a + b;
    EXPECT_NEAR(3.0, c.toDouble(), 0.001);
    EXPECT_NEAR(1.0, a.toDouble(), 0.001);
}

TEST(TzFixed, OperatorSub)
{
    TzFixed a(5.0, 16), b(2.0, 16);
    EXPECT_NEAR(3.0, (a - b).toDouble(), 0.001);
}

TEST(TzFixed, OperatorMul)
{
    TzFixed a(3.0, 16), b(4.0, 16);
    EXPECT_NEAR(12.0, (a * b).toDouble(), 0.01);
}

TEST(TzFixed, OperatorDiv)
{
    TzFixed a(10.0, 16), b(4.0, 16);
    EXPECT_NEAR(2.5, (a / b).toDouble(), 0.01);
}

TEST(TzFixed, EqualityOperators)
{
    TzFixed a(3.14, 16), b(3.14, 16), c(2.0, 16);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(TzFixed, OrderingOperators)
{
    TzFixed small(1.0, 16), large(2.0, 16);
    EXPECT_TRUE(small < large);
    EXPECT_TRUE(large > small);
    EXPECT_TRUE(small <= small);
    EXPECT_TRUE(small >= small);
    EXPECT_FALSE(large < small);
    EXPECT_FALSE(small > large);
}

TEST(TzFixed, CompareNegativeWithPositive)
{
    TzFixed neg(-1.0, 16), pos(1.0, 16);
    EXPECT_TRUE(neg < pos);
    EXPECT_FALSE(neg > pos);
}

TEST(TzFixed, Swap)
{
    TzFixed a(1.0, 16), b(2.0, 16);
    a.swap(b);
    EXPECT_NEAR(2.0, a.toDouble(), 0.001);
    EXPECT_NEAR(1.0, b.toDouble(), 0.001);
}

TEST(TzFixed, ZeroArithmetic)
{
    TzFixed a(3.14, 16), zero(16);
    EXPECT_NEAR(3.14, (a + zero).toDouble(), 0.001);
    EXPECT_NEAR(0.0,  (a * zero).toDouble(), 0.001);
}

TEST(TzFixed, HighPrecision)
{
    EXPECT_NEAR(0.123456789, TzFixed(0.123456789, 64).toDouble(), 1e-9);
}

TEST(TzFixed, NegativeArithmetic)
{
    TzFixed a(-3.0, 16), b(1.5, 16);
    EXPECT_NEAR(-1.5, (a + b).toDouble(), 0.001);
}
