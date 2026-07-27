/**
 * @file ups_model_spec_values_test.cpp
 * @brief Unit-тесты разбора допустимых значений параметров ИБП.
 *
 * Проверяется:
 *  - пустое значение normal
 *  - корректные и некорректные диапазоны
 *  - корректные и некорректные перечисления
 *  - значения режима байпаса
 *
 * Используются временные INI-файлы, создаваемые прямо в тестах.
 */

#include "ups_model_spec_test_base.h"

#include <string>

class UpsModelSpecValuesTest : public UpsModelSpecTestBase {};

#if (1)  // Пограничные случаи parseNormal

// Тест 1.1: normal пустой
TEST_F(UpsModelSpecValuesTest, ParseNormal_EmptyValue_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0
param1.oid = 1.3.6.1.2.1.1.1
param1.normal =
)");
    expectLoadFailure(ini, "TEST", "invalid normal for parameter param1: normal is empty");
}

// Тест 1.2: Корректный bypass не скрывает некорректный normal
TEST_F(UpsModelSpecValuesTest, ParseNormal_InvalidValueWithBypass_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = abc
param1.bypass = 6
)");
    expectLoadFailure(
        ini, "TEST", "invalid normal for parameter param1: enum contains non-numeric value");
}
#endif

#if (1)  // Пограничные случаи parseRange

// Тест 2.1: диапазон содержит более одного ".."
TEST_F(UpsModelSpecValuesTest, ParseRange_DoubleDots_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = 1..2..3
)");
    expectLoadFailure(ini, "TEST");
}

// Тест 2.2: диапазон с пустой границей
TEST_F(UpsModelSpecValuesTest, ParseRange_EmptyBound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = ..10
)");
    expectLoadFailure(ini, "TEST");
}

// Тест 2.3: диапазон с нечисловым значением
TEST_F(UpsModelSpecValuesTest, ParseRange_NonNumeric_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = a..10
)");
    expectLoadFailure(
        ini, "TEST", "invalid normal for parameter param1: range contains non-numeric value");
}

// Тест 2.4: диапазон с нечисловым суффиксом
TEST_F(UpsModelSpecValuesTest, ParseRange_NonNumericSuffix_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0
param1.oid = 1.2.3
param1.normal = 12a..34
)");
    expectLoadFailure(
        ini, "TEST", "invalid normal for parameter param1: range contains non-numeric value");
}
#endif

#if (1)  // Пограничные случаи parseEnumValues

// Тест 3.1: enum содержит пустое значение
TEST_F(UpsModelSpecValuesTest, ParseEnum_EmptyToken_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
param1.oid = 1.3.6.1.2.1.1.1
param1.normal = 1,,2
)");
    expectLoadFailure(ini, "TEST");
}

// Тест 3.2: enum состоит только из пробелов
TEST_F(UpsModelSpecValuesTest, ParseEnum_OnlySpaces_ReturnsError) {
    const std::string ini = m_tempIniFiles.write("[TEST]\n"
                                         "modelName = TEST_UPS\n"
                                         "param1.oid = 1.3.6.1.2.1.1.1\n"
                                         "param1.normal =    \n");
    expectLoadFailure(ini, "TEST");
}
#endif

#if (1)  // Формат нормальных значений

// Тест 6.1: Загрузка невозможна, если диапазон normal имеет некорректный формат
TEST_F(UpsModelSpecValuesTest, Load_NormalRangeInvalidSyntax_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200-240
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}

// Тест 6.2: Загрузка невозможна, если диапазон normal задан как min > max
TEST_F(UpsModelSpecValuesTest, Load_NormalRangeMinGreaterThanMax_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 260..200
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}

// Тест 6.3: Загрузка невозможна, если normal содержит нечисловое значение
TEST_F(UpsModelSpecValuesTest, Load_NormalEnumContainsNonNumeric_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
outputStatus.oid = 1.2.3.4
outputStatus.normal = 2,ok,4
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}

// Тест 6.4: Корректный диапазон normal успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_NormalRangeValid_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..259
)");
    expectLoadSuccess(ini, "TestUPS");

    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("inputVoltage");
    ASSERT_NE(it, params.end());
    const ups::NormalValueSpec& normal = it->second.normal;
    EXPECT_TRUE(normal.isRange);
    EXPECT_EQ(normal.min, 200u);
    EXPECT_EQ(normal.max, 259u);
}

// Тест 6.5: Корректное перечисление normal успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_NormalEnumValid_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
outputStatus.oid = 1.2.3.4
outputStatus.normal = 2,4,6
)");
    expectLoadSuccess(ini, "TestUPS");

    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("outputStatus");
    ASSERT_NE(it, params.end());
    const ups::NormalValueSpec& normal = it->second.normal;
    EXPECT_FALSE(normal.isRange);
    ASSERT_EQ(normal.values.size(), 3u);
    EXPECT_EQ(normal.values[0], 2u);
    EXPECT_EQ(normal.values[1], 4u);
    EXPECT_EQ(normal.values[2], 6u);
}
#endif

#if (1)  // Режим байпаса

// Тест 9.1: bypass — корректный enum
TEST_F(UpsModelSpecValuesTest, Load_BypassEnumValid_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
outputStatus.oid = 1.2.3.4
outputStatus.normal = 1,2,3
outputStatus.bypass = 6,9,10
)");
    expectLoadSuccess(ini, "TestUPS");

    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("outputStatus");
    ASSERT_NE(it, params.end());
    const auto& bypass = it->second.bypass;
    ASSERT_EQ(bypass.size(), 3u);
    EXPECT_EQ(bypass[0], 6u);
    EXPECT_EQ(bypass[1], 9u);
    EXPECT_EQ(bypass[2], 10u);
}

// Тест 9.2: bypass — одно значение (допустимо)
TEST_F(UpsModelSpecValuesTest, Load_BypassSingleValue_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
outputStatus.oid = 1.2.3.4
outputStatus.normal = 1
outputStatus.bypass = 6
)");
    expectLoadSuccess(ini, "TestUPS");

    const auto& params = m_spec.parameters();
    const auto it = params.find("outputStatus");
    ASSERT_NE(it, params.end());
    const auto& bypass = it->second.bypass;
    ASSERT_EQ(bypass.size(), 1u);
    EXPECT_EQ(bypass[0], 6u);
}

// Тест 9.3: bypass содержит нечисловое значение - ошибка
TEST_F(UpsModelSpecValuesTest, Load_BypassInvalidNonNumeric_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

outputStatus.oid = 1.2.3.4
outputStatus.normal = 1
outputStatus.bypass = 6,abc
)");
    expectLoadFailure(ini, "TestUPS", "bypass");
}
#endif
