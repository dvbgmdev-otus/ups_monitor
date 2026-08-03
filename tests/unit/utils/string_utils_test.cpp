/**
 * @file string_utils_test.cpp
 * @brief Unit-тесты утилит обработки строк.
 */
#include "string_utils.h"

#include <gtest/gtest.h>

#if (1)  // Удаление внешних пробельных символов

// Тест 1.1: Пустая строка остаётся пустой
TEST(StringUtilsTest, Trim_EmptyString_ReturnsEmptyString) {
    EXPECT_TRUE(utils::trim("").empty());
}

// Тест 1.2: Строка только из пробельных символов становится пустой
TEST(StringUtilsTest, Trim_WhitespaceOnly_ReturnsEmptyString) {
    EXPECT_TRUE(utils::trim(" \t\r\n").empty());
}

// Тест 1.3: Строка без внешних пробелов не изменяется
TEST(StringUtilsTest, Trim_NoOuterWhitespace_ReturnsSameString) {
    EXPECT_EQ(utils::trim("ups_monitor"), "ups_monitor");
}

// Тест 1.4: Пробельные символы слева удаляются
TEST(StringUtilsTest, Trim_LeadingWhitespace_RemovesWhitespace) {
    EXPECT_EQ(utils::trim(" \t\r\nups_monitor"), "ups_monitor");
}

// Тест 1.5: Пробельные символы справа удаляются
TEST(StringUtilsTest, Trim_TrailingWhitespace_RemovesWhitespace) {
    EXPECT_EQ(utils::trim("ups_monitor \t\r\n"), "ups_monitor");
}

// Тест 1.6: Пробельные символы с обеих сторон удаляются
TEST(StringUtilsTest, Trim_OuterWhitespace_RemovesWhitespace) {
    EXPECT_EQ(utils::trim(" \tups_monitor\r\n"), "ups_monitor");
}

// Тест 1.7: Внутренние пробельные символы сохраняются
TEST(StringUtilsTest, Trim_InnerWhitespace_PreservesWhitespace) {
    EXPECT_EQ(utils::trim(" \tUPS monitor status\r\n"), "UPS monitor status");
}

#endif
