#include <gtest/gtest.h>
#include <cmath>
#include <cstring>

#define TZBIGINT_IMPLEMENTATION
#include "../src/tzbigint.h"
#define TZFIXED_IMPLEMENTATION
#include "../src/tzfixed.h"

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
    EXPECT_EQ(result, nullptr); // should return NULL on error
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

    EXPECT_EQ(0, fixed_cmp(a, b)); // equal after rescaling

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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
