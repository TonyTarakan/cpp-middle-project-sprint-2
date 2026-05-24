#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <string_view>

#include "scan.hpp"

#define ASSERT_SCAN_OK(result) ASSERT_TRUE((result).has_value()) << (result).error().message
#define ASSERT_SCAN_ERR(result) ASSERT_FALSE((result).has_value())

//
// {d} - целые числа
//

TEST(ScanInt, SingleInt32) {
    auto r = stdx::scan<int32_t>("42", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), 42);
}

TEST(ScanInt, NegativeInt32) {
    auto r = stdx::scan<int32_t>("-7", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), -7);
}

TEST(ScanInt, Int8Min) {
    auto r = stdx::scan<int8_t>("-128", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), int8_t{-128});
}

TEST(ScanInt, Int64Large) {
    auto r = stdx::scan<int64_t>("9223372036854775807", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), INT64_MAX);
}

//
// {u} - беззнаковые числа
//

TEST(ScanUint, SingleUint32) {
    auto r = stdx::scan<uint32_t>("100", "{u}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), 100u);
}

TEST(ScanUint, Uint64Large) {
    auto r = stdx::scan<uint64_t>("18446744073709551615", "{u}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), UINT64_MAX);
}

TEST(ScanUint, Uint8Zero) {
    auto r = stdx::scan<uint8_t>("0", "{u}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), uint8_t{0});
}

//
// {f} - числа с плавающей точкой
//

TEST(ScanFloat, SingleFloat) {
    auto r = stdx::scan<float>("3.14", "{f}");
    ASSERT_SCAN_OK(r);
    EXPECT_FLOAT_EQ(std::get<0>(r->values()), 3.14f);
}

TEST(ScanFloat, SingleDouble) {
    auto r = stdx::scan<double>("2.718281828", "{f}");
    ASSERT_SCAN_OK(r);
    EXPECT_DOUBLE_EQ(std::get<0>(r->values()), 2.718281828);
}

TEST(ScanFloat, NegativeDouble) {
    auto r = stdx::scan<double>("-0.001", "{f}");
    ASSERT_SCAN_OK(r);
    EXPECT_DOUBLE_EQ(std::get<0>(r->values()), -0.001);
}

//
// {s} - строки
//

TEST(ScanString, StdString) {
    auto r = stdx::scan<std::string>("hello", "{s}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), "hello");
}

TEST(ScanString, StringView) {
    auto r = stdx::scan<std::string_view>("world", "{s}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), "world");
}

TEST(ScanString, EmptyString) {
    auto r = stdx::scan<std::string>("", "{s}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), "");
}

//
// Variadic
//

TEST(ScanMulti, IntAndString) {
    auto r = stdx::scan<int32_t, std::string>("42 hello", "{d} {s}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), 42);
    EXPECT_EQ(std::get<1>(r->values()), "hello");
}

TEST(ScanMulti, ThreeTypes) {
    auto r = stdx::scan<int32_t, double, std::string>("10 3.14 word", "{d} {f} {s}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), 10);
    EXPECT_DOUBLE_EQ(std::get<1>(r->values()), 3.14);
    EXPECT_EQ(std::get<2>(r->values()), "word");
}

TEST(ScanMulti, UintAndFloat) {
    auto r = stdx::scan<uint16_t, float>("255 1.5", "{u} {f}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), uint16_t{255});
    EXPECT_FLOAT_EQ(std::get<1>(r->values()), 1.5f);
}

TEST(ScanMulti, LiteralSeparator) {
    auto r = stdx::scan<int32_t, int32_t>("x=3,y=7", "x={d},y={d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), 3);
    EXPECT_EQ(std::get<1>(r->values()), 7);
}

//
// cv-квалификация
//

TEST(ScanCvQualified, ConstInt32) {
    auto r = stdx::scan<const int32_t>("5", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), 5);
}

TEST(ScanCvQualified, VolatileDouble) {
    auto r = stdx::scan<volatile double>("1.0", "{f}");
    ASSERT_SCAN_OK(r);
    EXPECT_DOUBLE_EQ(std::get<0>(r->values()), 1.0);
}

//
// Несоответствие спецификатора и типа
//

TEST(ScanError, StringSpecifierForInt) {
    auto r = stdx::scan<int32_t>("hello", "{s}");
    ASSERT_SCAN_ERR(r);
    EXPECT_FALSE(r.error().message.empty());
}

TEST(ScanError, IntSpecifierForString) {
    auto r = stdx::scan<std::string>("42", "{d}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, FloatSpecifierForUint) {
    auto r = stdx::scan<uint32_t>("3.14", "{f}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, UintSpecifierForFloat) {
    auto r = stdx::scan<double>("42", "{u}");
    ASSERT_SCAN_ERR(r);
}

//
// Некорректные входные данные
//

TEST(ScanError, NonNumericForInt) {
    auto r = stdx::scan<int32_t>("abc", "{d}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, NonNumericForUint) {
    auto r = stdx::scan<uint32_t>("xyz", "{u}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, NonNumericForFloat) {
    auto r = stdx::scan<double>("not_a_float", "{f}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, NegativeForUint) {
    auto r = stdx::scan<uint32_t>("-1", "{u}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, FormatLiteralMismatch) {
    auto r = stdx::scan<int32_t>("x=5", "y={d}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanError, TooFewPlaceholders) {
    // Два типа, но только один плейсхолдер
    auto r = stdx::scan<int32_t, int32_t>("1 2", "{d}");
    ASSERT_SCAN_ERR(r);
}

//
// Граничные значения
//

TEST(ScanBoundary, Int32Max) {
    auto r = stdx::scan<int32_t>("2147483647", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), INT32_MAX);
}

TEST(ScanBoundary, Int32Min) {
    auto r = stdx::scan<int32_t>("-2147483648", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), INT32_MIN);
}

TEST(ScanBoundary, Uint16Max) {
    auto r = stdx::scan<uint16_t>("65535", "{u}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), uint16_t{65535});
}

TEST(ScanBoundary, OverflowInt8) {
    // 200 > INT8_MAX (127)
    auto r = stdx::scan<int8_t>("200", "{d}");
    ASSERT_SCAN_ERR(r);
}

TEST(ScanBoundary, OverflowUint8) {
    // 300 > UINT8_MAX (255)
    auto r = stdx::scan<uint8_t>("300", "{u}");
    ASSERT_SCAN_ERR(r);
}

//
// scan_error сообщение
//

TEST(ScanErrorMessage, NonEmptyOnBadInput) {
    auto r = stdx::scan<int32_t>("garbage", "{d}");
    ASSERT_SCAN_ERR(r);
    EXPECT_FALSE(r.error().message.empty());
}

TEST(ScanErrorMessage, NonEmptyOnSpecifierMismatch) {
    auto r = stdx::scan<std::string>("42", "{d}");
    ASSERT_SCAN_ERR(r);
    EXPECT_FALSE(r.error().message.empty());
}

//
// Все числовые типы - smoke-тесты
//

TEST(ScanAllTypes, Int16) {
    auto r = stdx::scan<int16_t>("1000", "{d}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), int16_t{1000});
}

TEST(ScanAllTypes, Uint64) {
    auto r = stdx::scan<uint64_t>("123456789", "{u}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), uint64_t{123456789});
}

TEST(ScanAllTypes, Uint8) {
    auto r = stdx::scan<uint8_t>("255", "{u}");
    ASSERT_SCAN_OK(r);
    EXPECT_EQ(std::get<0>(r->values()), uint8_t{255});
}
