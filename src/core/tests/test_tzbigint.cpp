#include <gtest/gtest.h>
#include <cmath>
#include <stdexcept>
#include <string>
#include <cstring>

#include "../src/tzbigint.h"

#include <loom/tzbigint.hpp>

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

TEST(TzBigInt, DefaultConstructorIsZero)
{
    TzBigInt a;
    EXPECT_TRUE(a.isZero());
    EXPECT_FALSE(a.isNegative());
}

TEST(TzBigInt, ConstructFromInt64Positive)
{
    TzBigInt a(12345LL);
    EXPECT_FALSE(a.isZero());
    EXPECT_FALSE(a.isNegative());
    EXPECT_DOUBLE_EQ(12345.0, a.toDouble());
}

TEST(TzBigInt, ConstructFromInt64Negative)
{
    TzBigInt a(-9876LL);
    EXPECT_TRUE(a.isNegative());
    EXPECT_DOUBLE_EQ(-9876.0, a.toDouble());
}

TEST(TzBigInt, ConstructFromInt64Zero)
{
    TzBigInt a(0LL);
    EXPECT_TRUE(a.isZero());
}

TEST(TzBigInt, ConstructFromCString)
{
    TzBigInt a("1234567890123456789");
    EXPECT_EQ("1234567890123456789", a.toString());
}

TEST(TzBigInt, ConstructFromStdString)
{
    std::string s("9876543210987654321");
    TzBigInt a(s);
    EXPECT_EQ(s, a.toString());
}

TEST(TzBigInt, ConstructFromNegativeCString)
{
    TzBigInt a("-42");
    EXPECT_TRUE(a.isNegative());
    EXPECT_DOUBLE_EQ(-42.0, a.toDouble());
}

TEST(TzBigInt, CopyConstructor)
{
    TzBigInt a(100LL);
    TzBigInt b(a);
    EXPECT_DOUBLE_EQ(a.toDouble(), b.toDouble());
    b += TzBigInt(1LL);
    EXPECT_DOUBLE_EQ(100.0, a.toDouble());
    EXPECT_DOUBLE_EQ(101.0, b.toDouble());
}

TEST(TzBigInt, MoveConstructor)
{
    TzBigInt a(999LL);
    TzBigInt b(std::move(a));
    EXPECT_DOUBLE_EQ(999.0, b.toDouble());
}

TEST(TzBigInt, CopyAssignment)
{
    TzBigInt a(7LL);
    TzBigInt b(3LL);
    b = a;
    EXPECT_DOUBLE_EQ(7.0, b.toDouble());
    b += TzBigInt(1LL);
    EXPECT_DOUBLE_EQ(7.0, a.toDouble());
}

TEST(TzBigInt, CopyAssignmentSelf)
{
    TzBigInt a(42LL);
    a = a;
    EXPECT_DOUBLE_EQ(42.0, a.toDouble());
}

TEST(TzBigInt, MoveAssignment)
{
    TzBigInt a(555LL);
    TzBigInt b;
    b = std::move(a);
    EXPECT_DOUBLE_EQ(555.0, b.toDouble());
}

TEST(TzBigInt, SetNegative)
{
    TzBigInt a(50LL);
    a.setNegative(true);
    EXPECT_TRUE(a.isNegative());
    EXPECT_DOUBLE_EQ(-50.0, a.toDouble());

    a.setNegative(false);
    EXPECT_FALSE(a.isNegative());
    EXPECT_DOUBLE_EQ(50.0, a.toDouble());
}

TEST(TzBigInt, ToString)
{
    EXPECT_EQ("1234567890", TzBigInt(1234567890LL).toString());
    EXPECT_EQ("-1234567890", TzBigInt(-1234567890LL).toString());
    EXPECT_EQ("0", TzBigInt(0LL).toString());
}

TEST(TzBigInt, AddAssign)
{
    TzBigInt a(100LL), b(200LL);
    a += b;
    EXPECT_DOUBLE_EQ(300.0, a.toDouble());
}

TEST(TzBigInt, SubAssign)
{
    TzBigInt a(300LL), b(100LL);
    a -= b;
    EXPECT_DOUBLE_EQ(200.0, a.toDouble());
}

TEST(TzBigInt, SubAssignNegativeResult)
{
    TzBigInt a(50LL), b(100LL);
    a -= b;
    EXPECT_TRUE(a.isNegative());
    EXPECT_DOUBLE_EQ(-50.0, a.toDouble());
}

TEST(TzBigInt, MulAssign)
{
    TzBigInt a(25LL), b(4LL);
    a *= b;
    EXPECT_DOUBLE_EQ(100.0, a.toDouble());
}

TEST(TzBigInt, DivAssign)
{
    TzBigInt a(100LL), b(4LL);
    a /= b;
    EXPECT_DOUBLE_EQ(25.0, a.toDouble());
}

TEST(TzBigInt, DivAssignByZeroThrows)
{
    TzBigInt a(100LL), zero;
    EXPECT_THROW(a /= zero, std::runtime_error);
}

TEST(TzBigInt, ModAssign)
{
    TzBigInt a(17LL), b(5LL);
    a %= b;
    EXPECT_DOUBLE_EQ(2.0, a.toDouble());
}

TEST(TzBigInt, ModAssignByZeroThrows)
{
    TzBigInt a(100LL), zero;
    EXPECT_THROW(a %= zero, std::runtime_error);
}

TEST(TzBigInt, ShlAssign)
{
    TzBigInt a(5LL);
    a <<= 3;
    EXPECT_DOUBLE_EQ(40.0, a.toDouble());
}

TEST(TzBigInt, ShrAssign)
{
    TzBigInt a(40LL);
    a >>= 3;
    EXPECT_DOUBLE_EQ(5.0, a.toDouble());
}

TEST(TzBigInt, OperatorAdd)
{
    TzBigInt a(10LL), b(3LL);
    TzBigInt c = a + b;
    EXPECT_DOUBLE_EQ(13.0, c.toDouble());
    EXPECT_DOUBLE_EQ(10.0, a.toDouble());
}

TEST(TzBigInt, OperatorSub)
{
    TzBigInt a(10LL), b(3LL);
    EXPECT_DOUBLE_EQ(7.0, (a - b).toDouble());
}

TEST(TzBigInt, OperatorMul)
{
    TzBigInt a(6LL), b(7LL);
    EXPECT_DOUBLE_EQ(42.0, (a * b).toDouble());
}

TEST(TzBigInt, OperatorDiv)
{
    TzBigInt a(42LL), b(7LL);
    EXPECT_DOUBLE_EQ(6.0, (a / b).toDouble());
}

TEST(TzBigInt, OperatorMod)
{
    TzBigInt a(17LL), b(5LL);
    EXPECT_DOUBLE_EQ(2.0, (a % b).toDouble());
}

TEST(TzBigInt, OperatorShl)
{
    TzBigInt a(1LL);
    TzBigInt b = a << 10;
    EXPECT_DOUBLE_EQ(1024.0, b.toDouble());
    EXPECT_DOUBLE_EQ(1.0, a.toDouble());
}

TEST(TzBigInt, OperatorShr)
{
    EXPECT_DOUBLE_EQ(1.0, (TzBigInt(1024LL) >> 10).toDouble());
}

TEST(TzBigInt, EqualityOperators)
{
    TzBigInt a(42LL), b(42LL), c(43LL);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(TzBigInt, OrderingOperators)
{
    TzBigInt small(10LL), large(20LL);
    EXPECT_TRUE(small < large);
    EXPECT_TRUE(large > small);
    EXPECT_TRUE(small <= small);
    EXPECT_TRUE(small >= small);
    EXPECT_FALSE(large < small);
    EXPECT_FALSE(small > large);
}

TEST(TzBigInt, CompareNegativeWithPositive)
{
    TzBigInt neg(-5LL), pos(5LL);
    EXPECT_TRUE(neg < pos);
    EXPECT_FALSE(neg > pos);
}

TEST(TzBigInt, Swap)
{
    TzBigInt a(1LL), b(2LL);
    a.swap(b);
    EXPECT_DOUBLE_EQ(2.0, a.toDouble());
    EXPECT_DOUBLE_EQ(1.0, b.toDouble());
}

TEST(TzBigInt, LargeNumberRoundTrip)
{
    const std::string big = "1234567890123456789012345678901234567890";
    EXPECT_EQ(big, TzBigInt(big).toString());
}

TEST(TzBigInt, LargeShift)
{
    TzBigInt a(1LL);
    a <<= 64;
    EXPECT_NEAR(1.8446744e19, a.toDouble(), 1e13);
}
