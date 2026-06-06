#include <gtest/gtest.h>
#include <cmath>
#include <string>
#include <cstring>

#define TZBIGINT_IMPLEMENTATION
#include "../src/tzbigint.h"

class BigIntTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        zero = bigint_from_int64(0);
        positive = bigint_from_int64(12345);
        negative = bigint_from_int64(-67890);
        max64 = bigint_from_int64(INT64_MAX);
        min64 = bigint_from_int64(INT64_MIN);
    }

    void TearDown() override
    {
        bigint_destroy(zero);
        bigint_destroy(positive);
        bigint_destroy(negative);
        bigint_destroy(max64);
        bigint_destroy(min64);
    }

    bigint_t* zero;
    bigint_t* positive;
    bigint_t* negative;
    bigint_t* max64;
    bigint_t* min64;
};

TEST_F(BigIntTest, CreateFromInt64)
{
    bigint_t* num = bigint_from_int64(42);
    ASSERT_NE(num, nullptr);
    EXPECT_FALSE(bigint_is_zero(num));
    EXPECT_FALSE(bigint_is_negative(num));
    bigint_destroy(num);
}

TEST_F(BigIntTest, CreateFromString)
{
    bigint_t* num = bigint_from_string("12345678901234567890");
    ASSERT_NE(num, nullptr);
    EXPECT_FALSE(bigint_is_zero(num));

    char* str = bigint_to_string(num);
    EXPECT_STREQ("12345678901234567890", str);
    free(str);
    bigint_destroy(num);
}

TEST_F(BigIntTest, CreateFromNegativeString)
{
    bigint_t* num = bigint_from_string("-9876543210");
    ASSERT_NE(num, nullptr);
    EXPECT_TRUE(bigint_is_negative(num));
    bigint_destroy(num);
}

TEST_F(BigIntTest, Clone)
{
    bigint_t* clone = bigint_clone(positive);
    ASSERT_NE(clone, nullptr);
    EXPECT_FALSE(bigint_is_zero(clone));
    EXPECT_EQ(0, bigint_cmp(positive, clone));
    bigint_destroy(clone);
}

TEST_F(BigIntTest, IsZero)
{
    EXPECT_TRUE(bigint_is_zero(zero));
    EXPECT_FALSE(bigint_is_zero(positive));
    EXPECT_FALSE(bigint_is_zero(negative));
}

TEST_F(BigIntTest, IsNegative)
{
    EXPECT_FALSE(bigint_is_negative(zero));
    EXPECT_FALSE(bigint_is_negative(positive));
    EXPECT_TRUE(bigint_is_negative(negative));
}

TEST_F(BigIntTest, SetNegative)
{
    bigint_t* num = bigint_from_int64(100);
    EXPECT_FALSE(bigint_is_negative(num));

    bigint_set_negative(num, 1);
    EXPECT_TRUE(bigint_is_negative(num));

    bigint_set_negative(num, 0);
    EXPECT_FALSE(bigint_is_negative(num));

    bigint_destroy(num);
}

TEST_F(BigIntTest, CompareEqual)
{
    bigint_t* a = bigint_from_int64(12345);
    bigint_t* b = bigint_from_int64(12345);

    EXPECT_EQ(0, bigint_cmp(a, b));
    EXPECT_EQ(0, bigint_cmp_abs(a, b));

    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, CompareLess)
{
    bigint_t* small = bigint_from_int64(100);
    bigint_t* large = bigint_from_int64(200);

    EXPECT_LT(bigint_cmp(small, large), 0);
    EXPECT_GT(bigint_cmp(large, small), 0);

    bigint_destroy(small);
    bigint_destroy(large);
}

TEST_F(BigIntTest, CompareNegative)
{
    bigint_t* neg = bigint_from_int64(-50);
    bigint_t* pos = bigint_from_int64(30);

    EXPECT_LT(bigint_cmp(neg, pos), 0);
    EXPECT_GT(bigint_cmp(pos, neg), 0);

    bigint_destroy(neg);
    bigint_destroy(pos);
}

TEST_F(BigIntTest, CompareAbs)
{
    bigint_t* neg = bigint_from_int64(-100);
    bigint_t* pos = bigint_from_int64(50);

    EXPECT_GT(bigint_cmp_abs(neg, pos), 0);  // |-100| > 50

    bigint_destroy(neg);
    bigint_destroy(pos);
}

TEST_F(BigIntTest, ToDouble)
{
    bigint_t* num = bigint_from_int64(12345);
    EXPECT_DOUBLE_EQ(12345.0, bigint_to_double(num));
    bigint_destroy(num);
}

TEST_F(BigIntTest, ToDoubleNegative)
{
    bigint_t* num = bigint_from_int64(-54321);
    EXPECT_DOUBLE_EQ(-54321.0, bigint_to_double(num));
    bigint_destroy(num);
}

TEST_F(BigIntTest, ToDoubleLarge)
{
    bigint_t* num = bigint_from_string("9999999999999999999");
    double result = bigint_to_double(num);
    EXPECT_GT(result, 9.9e18);
    EXPECT_LE(result, 1.0e19);
    bigint_destroy(num);
}

TEST_F(BigIntTest, ToString)
{
    const char* expected = "1234567890";
    bigint_t* num = bigint_from_string(expected);
    char* str = bigint_to_string(num);
    EXPECT_STREQ(expected, str);
    free(str);
    bigint_destroy(num);
}

TEST_F(BigIntTest, Addition)
{
    bigint_t* a = bigint_from_int64(100);
    bigint_t* b = bigint_from_int64(200);

    EXPECT_EQ(0, bigint_add_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("300", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, AdditionWithNegative)
{
    bigint_t* a = bigint_from_int64(100);
    bigint_t* b = bigint_from_int64(-50);

    EXPECT_EQ(0, bigint_add_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("50", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, Subtraction)
{
    bigint_t* a = bigint_from_int64(300);
    bigint_t* b = bigint_from_int64(100);

    EXPECT_EQ(0, bigint_sub_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("200", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, SubtractionResultNegative)
{
    bigint_t* a = bigint_from_int64(50);
    bigint_t* b = bigint_from_int64(100);

    EXPECT_EQ(0, bigint_sub_assign(a, b));

    EXPECT_TRUE(bigint_is_negative(a));
    char* result = bigint_to_string(a);
    EXPECT_STREQ("-50", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, Multiplication)
{
    bigint_t* a = bigint_from_int64(25);
    bigint_t* b = bigint_from_int64(4);

    EXPECT_EQ(0, bigint_mul_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("100", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, MultiplicationLarge)
{
    bigint_t* a = bigint_from_string("1000000");
    bigint_t* b = bigint_from_string("1000000");

    EXPECT_EQ(0, bigint_mul_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("1000000000000", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, MultiplicationNegative)
{
    bigint_t* a = bigint_from_int64(-10);
    bigint_t* b = bigint_from_int64(5);

    EXPECT_EQ(0, bigint_mul_assign(a, b));

    EXPECT_TRUE(bigint_is_negative(a));
    char* result = bigint_to_string(a);
    EXPECT_STREQ("-50", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, Division)
{
    bigint_t* a = bigint_from_int64(100);
    bigint_t* b = bigint_from_int64(4);

    EXPECT_EQ(0, bigint_div_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("25", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, DivisionByZero)
{
    bigint_t* a        = bigint_from_int64(100);
    bigint_t* zero_div = bigint_from_int64(0);

    EXPECT_EQ(-1, bigint_div_assign(a, zero_div));

    bigint_destroy(a);
    bigint_destroy(zero_div);
}

TEST_F(BigIntTest, Modulo)
{
    bigint_t* a = bigint_from_int64(17);
    bigint_t* b = bigint_from_int64(5);

    EXPECT_EQ(0, bigint_mod_assign(a, b));

    char* result = bigint_to_string(a);
    EXPECT_STREQ("2", result);

    free(result);
    bigint_destroy(a);
    bigint_destroy(b);
}

TEST_F(BigIntTest, ModuloByZero)
{
    bigint_t* a        = bigint_from_int64(100);
    bigint_t* zero_mod = bigint_from_int64(0);

    EXPECT_EQ(-1, bigint_mod_assign(a, zero_mod));

    bigint_destroy(a);
    bigint_destroy(zero_mod);
}

TEST_F(BigIntTest, ShiftLeft)
{
    bigint_t* num = bigint_from_int64(5);
    bigint_shl(num, 3);  // 5 << 3 = 40

    char* result = bigint_to_string(num);
    EXPECT_STREQ("40", result);

    free(result);
    bigint_destroy(num);
}

TEST_F(BigIntTest, ShiftRight)
{
    bigint_t* num = bigint_from_int64(40);
    bigint_shr(num, 3);  // 40 >> 3 = 5

    char* result = bigint_to_string(num);
    EXPECT_STREQ("5", result);

    free(result);
    bigint_destroy(num);
}

TEST_F(BigIntTest, ShiftLeftLarge)
{
    bigint_t* num = bigint_from_string("1");
    bigint_shl(num, 64);  // 2^64

    double result = bigint_to_double(num);
    EXPECT_DOUBLE_EQ(pow(2.0, 64), result);

    bigint_destroy(num);
}

TEST(BigIntEdgeTest, ZeroFromString)
{
    bigint_t* num = bigint_from_string("0");
    EXPECT_TRUE(bigint_is_zero(num));
    bigint_destroy(num);
}

TEST(BigIntEdgeTest, NegativeZero)
{
    bigint_t* num = bigint_from_string("-0");
    EXPECT_TRUE(bigint_is_zero(num));
    EXPECT_FALSE(bigint_is_negative(num)); // normalized to zero
    bigint_destroy(num);
}

TEST(BigIntEdgeTest, MaxInt64)
{
    bigint_t* num = bigint_from_int64(INT64_MAX);
    EXPECT_DOUBLE_EQ((double)INT64_MAX, bigint_to_double(num));
    bigint_destroy(num);
}

TEST(BigIntEdgeTest, MinInt64)
{
    bigint_t* num = bigint_from_int64(INT64_MIN);
    EXPECT_DOUBLE_EQ((double)INT64_MIN, bigint_to_double(num));
    bigint_destroy(num);
}

TEST(BigIntEdgeTest, VeryLargeString)
{
    const char* large = "1234567890123456789012345678901234567890";
    bigint_t* num = bigint_from_string(large);
    ASSERT_NE(num, nullptr);
    char* result = bigint_to_string(num);
    EXPECT_STREQ(large, result);
    free(result);
    bigint_destroy(num);
}
