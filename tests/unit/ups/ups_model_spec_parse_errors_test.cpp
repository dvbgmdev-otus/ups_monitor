/**
 * @file ups_model_spec_parse_errors_test.cpp
 * @brief Unit-тесты внутренних ошибок парсинга UpsModelSpec.
 *
 * Проверяются пограничные сценарии:
 *  - пустое значение normal
 *  - некорректный синтаксис диапазона
 *  - некорректные enum-значения
 *
 * Используются временные INI-файлы, создаваемые прямо в тестах.
 */

#include "ups_model_spec.h"
#include "temp_ini_file.h"

#include <gtest/gtest.h>

#include <string>

class UpsModelSpecParseErrorsTest : public ::testing::Test {
protected:
    ups::UpsModelSpec m_spec;
    test::TempIniFileStorage m_tempIniFiles;

    void expectLoadFailure(const std::string& ini,
                           const std::string& section = "TEST",
                           const std::string& expected = "") {
        const bool ok = m_spec.load(ini, section);
        EXPECT_FALSE(ok);
        EXPECT_FALSE(m_spec.lastError().empty()) << "Error message must not be empty";
        if (!expected.empty()) {
            EXPECT_NE(m_spec.lastError().find(expected), std::string::npos)
                << m_spec.lastError();
        }
    }
};

#if (1)  // Пограничные случаи parseNormal

// Тест 1.1: normal пустой
TEST_F(UpsModelSpecParseErrorsTest, ParseNormal_EmptyValue_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal =
)");
    expectLoadFailure(ini, "TEST", "normal");
}
#endif

#if (1)  // Пограничные случаи parseRange

// Тест 2.1: диапазон содержит более одного ".."
TEST_F(UpsModelSpecParseErrorsTest, ParseRange_DoubleDots_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = 1..2..3
)");
    expectLoadFailure(ini, "TEST");
}

// Тест 2.2: диапазон с пустой границей
TEST_F(UpsModelSpecParseErrorsTest, ParseRange_EmptyBound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = ..10
)");
    expectLoadFailure(ini, "TEST");
}

// Тест 2.3: диапазон с нечисловым значением
TEST_F(UpsModelSpecParseErrorsTest, ParseRange_NonNumeric_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = a..10
)");
    expectLoadFailure(ini, "TEST", "without normal or bypass");
}

// Тест 2.4: диапазон с нечисловым суффиксом
TEST_F(UpsModelSpecParseErrorsTest, ParseRange_NonNumericSuffix_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.2.3
param1.normal = 12a..34
)");
    expectLoadFailure(ini, "TEST", "without normal or bypass");
}
#endif

#if (1)  // Пограничные случаи parseEnumValues

// Тест 3.1: enum содержит пустое значение
TEST_F(UpsModelSpecParseErrorsTest, ParseEnum_EmptyToken_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = 1,,2
)");
    expectLoadFailure(ini, "TEST");
}

// Тест 3.2: enum состоит только из пробелов
TEST_F(UpsModelSpecParseErrorsTest, ParseEnum_OnlySpaces_ReturnsError) {
    const std::string ini = m_tempIniFiles.write("[TEST]\n"
                                         "modelName = TEST_UPS\n"
                                         "param1.oid = 1.3.6.1.2.1.1.1\n"
                                         "param1.normal =    \n");
    expectLoadFailure(ini, "TEST");
}
#endif
