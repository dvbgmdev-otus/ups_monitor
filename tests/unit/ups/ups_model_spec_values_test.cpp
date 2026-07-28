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

#if (1)  // Часть 1 — Общая проверка normal

// Тест 1.1: Загрузка невозможна, если normal пустой
TEST_F(UpsModelSpecValuesTest, Load_EmptyNormal_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0

param1.oid = 1.3.6.1.2.1.1.1
param1.normal =
)");
    expectLoadFailure(ini, "TEST", "invalid normal for parameter param1: normal is empty");
}

// Тест 1.2: Загрузка невозможна, если normal состоит только из пробелов
TEST_F(UpsModelSpecValuesTest, Load_WhitespaceOnlyNormal_ReturnsError) {
    const std::string ini =
        m_tempIniFiles.write("[TEST]\n"
                             "modelName = TEST_UPS\n"
                             "modelName.oid = 1.3.6.1.2.1.1.2.0\n"
                             "\n"
                             "param1.oid = 1.3.6.1.2.1.1.1\n"
                             "param1.normal =    \n");
    expectLoadFailure(ini, "TEST", "invalid normal for parameter param1: normal is empty");
}

// Тест 1.3: Указанный normal должен быть корректным даже при наличии bypass
TEST_F(UpsModelSpecValuesTest, Load_InvalidNormalWithBypass_ReturnsError) {
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

#if (1)  // Часть 2 — Диапазон normal

// Тест 2.1: Загрузка невозможна, если диапазон содержит несколько разделителей
TEST_F(UpsModelSpecValuesTest, Load_RangeWithMultipleSeparators_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0

param1.oid = 1.3.6.1.2.1.1.1
param1.normal = 1..2..3
)");
    expectLoadFailure(ini, "TEST", "invalid normal for parameter param1: invalid range syntax");
}

// Тест 2.2: Загрузка невозможна, если нижняя граница диапазона отсутствует
TEST_F(UpsModelSpecValuesTest, Load_RangeWithEmptyLowerBound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0

param1.oid = 1.3.6.1.2.1.1.1
param1.normal = ..10
)");
    expectLoadFailure(ini, "TEST", "invalid normal for parameter param1: invalid range syntax");
}

// Тест 2.3: Загрузка невозможна, если верхняя граница диапазона отсутствует
TEST_F(UpsModelSpecValuesTest, Load_RangeWithEmptyUpperBound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.3.6.1.2.1.1.2.0

param1.oid = 1.3.6.1.2.1.1.1
param1.normal = 10..
)");
    expectLoadFailure(ini, "TEST", "invalid normal for parameter param1: invalid range syntax");
}

// Тест 2.4: Загрузка невозможна, если граница диапазона не является числом
TEST_F(UpsModelSpecValuesTest, Load_RangeWithNonNumericBound_ReturnsError) {
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

// Тест 2.5: Загрузка невозможна, если граница диапазона содержит нечисловой суффикс
TEST_F(UpsModelSpecValuesTest, Load_RangeWithNonNumericSuffix_ReturnsError) {
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

// Тест 2.6: Загрузка невозможна, если нижняя граница больше верхней
TEST_F(UpsModelSpecValuesTest, Load_RangeWithMinGreaterThanMax_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 260..200
)");
    expectLoadFailure(
        ini, "TestUPS", "invalid normal for parameter inputVoltage: range min greater than max");
}

// Тест 2.7: Загрузка невозможна, если граница диапазона отрицательная
TEST_F(UpsModelSpecValuesTest, Load_RangeWithNegativeBound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 0..-1
)");
    expectLoadFailure(ini, "TestUPS", "invalid normal for parameter inputVoltage");
}

// Тест 2.8: Загрузка невозможна, если граница диапазона превышает uint32_t
TEST_F(UpsModelSpecValuesTest, Load_RangeWithUint32Overflow_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 0..4294967296
)");
    expectLoadFailure(ini, "TestUPS", "invalid normal for parameter inputVoltage");
}

// Тест 2.9: Загрузка невозможна, если граница диапазона не помещается в unsigned long
TEST_F(UpsModelSpecValuesTest, Load_RangeWithUnsignedLongOverflow_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 0..99999999999999999999999999999999999999999999999999
)");
    expectLoadFailure(ini,
                      "TestUPS",
                      "invalid normal for parameter inputVoltage: range value exceeds uint32_t");
}

// Тест 2.10: Диапазон с одинаковыми границами успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_RangeWithEqualBounds_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 10..10
)");
    expectLoadSuccess(ini, "TestUPS");
    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("inputVoltage");
    ASSERT_NE(it, params.end());
    const ups::NormalValueSpec& normal = it->second.normal;
    EXPECT_TRUE(normal.isRange);
    EXPECT_EQ(normal.min, 10u);
    EXPECT_EQ(normal.max, 10u);
}

// Тест 2.11: Корректный диапазон успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_ValidRange_Succeeds) {
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
#endif

#if (1)  // Часть 3 — Перечисление normal

// Тест 3.1: Загрузка невозможна, если перечисление содержит пустой элемент
TEST_F(UpsModelSpecValuesTest, Load_EnumWithEmptyValue_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 1,,2
)");
    expectLoadFailure(
        ini, "TestUPS", "invalid normal for parameter batteryStatus: enum contains empty value");
}

// Тест 3.2: Загрузка невозможна, если перечисление содержит нечисловой элемент
TEST_F(UpsModelSpecValuesTest, Load_EnumWithNonNumericValue_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 2,ok,4
)");
    expectLoadFailure(ini,
                      "TestUPS",
                      "invalid normal for parameter batteryStatus: enum contains non-numeric value");
}

// Тест 3.3: Загрузка невозможна, если элемент содержит недопустимые символы
TEST_F(UpsModelSpecValuesTest, Load_EnumWithInvalidCharacters_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200-240
)");
    expectLoadFailure(
        ini,
        "TestUPS",
        "invalid normal for parameter inputVoltage: enum contains invalid characters");
}

// Тест 3.4: Загрузка невозможна, если перечисление заканчивается запятой
TEST_F(UpsModelSpecValuesTest, Load_EnumWithTrailingComma_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 1,2,
)");
    expectLoadFailure(ini, "TestUPS", "invalid normal for parameter batteryStatus");
}

// Тест 3.5: Загрузка невозможна, если перечисление содержит отрицательное значение
TEST_F(UpsModelSpecValuesTest, Load_EnumWithNegativeValue_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = -1
)");
    expectLoadFailure(ini, "TestUPS", "invalid normal for parameter batteryStatus");
}

// Тест 3.6: Загрузка невозможна, если значение перечисления превышает uint32_t
TEST_F(UpsModelSpecValuesTest, Load_EnumWithUint32Overflow_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 4294967296
)");
    expectLoadFailure(ini, "TestUPS", "invalid normal for parameter batteryStatus");
}

// Тест 3.7: Загрузка невозможна, если значение перечисления не помещается в unsigned long
TEST_F(UpsModelSpecValuesTest, Load_EnumWithUnsignedLongOverflow_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 99999999999999999999999999999999999999999999999999
)");
    expectLoadFailure(ini,
                      "TestUPS",
                      "invalid normal for parameter batteryStatus: enum value exceeds uint32_t");
}

// Тест 3.8: Перечисление из одного значения успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_SingleValueEnum_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 2
)");
    expectLoadSuccess(ini, "TestUPS");
    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("batteryStatus");
    ASSERT_NE(it, params.end());
    const ups::NormalValueSpec& normal = it->second.normal;
    EXPECT_FALSE(normal.isRange);
    ASSERT_EQ(normal.values.size(), 1u);
    EXPECT_EQ(normal.values[0], 2u);
}

// Тест 3.9: Перечисление из нескольких значений успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_MultipleValueEnum_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

batteryStatus.oid = 1.2.3.4
batteryStatus.normal = 2,4,6
)");
    expectLoadSuccess(ini, "TestUPS");
    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("batteryStatus");
    ASSERT_NE(it, params.end());
    const ups::NormalValueSpec& normal = it->second.normal;
    EXPECT_FALSE(normal.isRange);
    ASSERT_EQ(normal.values.size(), 3u);
    EXPECT_EQ(normal.values[0], 2u);
    EXPECT_EQ(normal.values[1], 4u);
    EXPECT_EQ(normal.values[2], 6u);
}
#endif

#if (1)  // Часть 4 — Режим байпаса

// Тест 4.1: Перечисление bypass из нескольких значений успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_MultipleValueBypass_Succeeds) {
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

// Тест 4.2: Перечисление bypass из одного значения успешно загружается
TEST_F(UpsModelSpecValuesTest, Load_SingleValueBypass_Succeeds) {
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
    ASSERT_EQ(params.size(), 1u);
    const auto it = params.find("outputStatus");
    ASSERT_NE(it, params.end());
    const auto& bypass = it->second.bypass;
    ASSERT_EQ(bypass.size(), 1u);
    EXPECT_EQ(bypass[0], 6u);
}

// Тест 4.3: Указанный bypass должен быть корректным даже при наличии normal
TEST_F(UpsModelSpecValuesTest, Load_InvalidBypassWithNormal_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

outputStatus.oid = 1.2.3.4
outputStatus.normal = 1
outputStatus.bypass = 6,abc
)");
    expectLoadFailure(
        ini,
        "TestUPS",
        "invalid bypass for parameter outputStatus: enum contains non-numeric value");
}

// Тест 4.4: Указанный bypass не может быть пустым даже при наличии normal
TEST_F(UpsModelSpecValuesTest, Load_EmptyBypassWithNormal_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

outputStatus.oid = 1.2.3.4
outputStatus.normal = 1
outputStatus.bypass =
)");
    expectLoadFailure(ini, "TestUPS", "invalid bypass for parameter outputStatus");
}
#endif
