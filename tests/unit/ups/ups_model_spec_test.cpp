/**
 * @file ups_model_spec_test.cpp
 * @brief Unit-тесты загрузки и проверки спецификации модели ИБП.
 *
 * Проверяется:
 *  - обработка отсутствующего файла и секции
 *  - проверка обязательных полей модели и параметров
 *  - разбор диапазонов и перечислений нормальных значений
 *  - загрузка нескольких параметров
 *  - разбор значений режима байпаса
 *
 * Используются временные INI-файлы, создаваемые прямо в тестах.
 */

#include "ups_model_spec.h"
#include "temp_ini_file.h"

#include <gtest/gtest.h>

#include <string>

class UpsModelSpecTest : public ::testing::Test {
protected:
    ups::UpsModelSpec m_spec;
    test::TempIniFileStorage m_tempIniFiles;

    // Проверяет, что загрузка завершилась ошибкой.
    void expectLoadFailure(const std::string& file,
                           const std::string& section,
                           const std::string& expected = "") {
        const bool ok = m_spec.load(file, section);
        EXPECT_FALSE(ok) << m_spec.lastError();
        EXPECT_FALSE(m_spec.lastError().empty());
        if (!expected.empty()) {
            EXPECT_NE(m_spec.lastError().find(expected), std::string::npos) << m_spec.lastError();
        }
    }

    // Проверяет, что загрузка завершилась успешно.
    void expectLoadSuccess(const std::string& file, const std::string& section) {
        const bool ok = m_spec.load(file, section);
        EXPECT_TRUE(ok) << m_spec.lastError();
        EXPECT_TRUE(m_spec.lastError().empty()) << m_spec.lastError();
    }
};

#if (1)  // Предварительные условия

// Тест 1.1: Загрузка спецификации невозможна, если INI-файл не существует
TEST_F(UpsModelSpecTest, Load_FileNotFound_ReturnsError) {
    expectLoadFailure("non_existing_file.ini", "APC");
}
#endif

#if (1)  // Проверка секции модели

// Тест 2.1: Загрузка невозможна, если секция модели отсутствует
TEST_F(UpsModelSpecTest, Load_SectionNotFound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3
inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "NON_EXISTING_MODEL", "section not found");
}
#endif

#if (1)  // Метаданные модели

// Тест 3.1: Загрузка невозможна, если в секции отсутствует modelName
TEST_F(UpsModelSpecTest, Load_ModelNameMissing_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
inputVoltage.oid = 1.2.3
inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "TestUPS", "modelName");
}
#endif

#if (1)  // Спецификация параметра

// Тест 4.1: Загрузка невозможна, если у параметра отсутствует .oid
TEST_F(UpsModelSpecTest, Load_ParamWithoutOid_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "TestUPS", "oid");
}
#endif

#if (1)  // Критерии нормального состояния

// Тест 5.1: Загрузка невозможна, если у параметра отсутствует .normal
TEST_F(UpsModelSpecTest, Load_ParamWithoutNormal_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3.4
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}
#endif

#if (1)  // Формат нормальных значений

// Тест 6.1: Загрузка невозможна, если диапазон normal имеет некорректный формат
TEST_F(UpsModelSpecTest, Load_NormalRangeInvalidSyntax_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200-240
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}

// Тест 6.2: Загрузка невозможна, если диапазон normal задан как min > max
TEST_F(UpsModelSpecTest, Load_NormalRangeMinGreaterThanMax_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 260..200
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}

// Тест 6.3: Загрузка невозможна, если normal содержит нечисловое значение
TEST_F(UpsModelSpecTest, Load_NormalEnumContainsNonNumeric_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

outputStatus.oid = 1.2.3.4
outputStatus.normal = 2,ok,4
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}

// Тест 6.4: Корректный диапазон normal успешно загружается
TEST_F(UpsModelSpecTest, Load_NormalRangeValid_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

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
TEST_F(UpsModelSpecTest, Load_NormalEnumValid_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

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

#if (1)  // Несколько параметров

// Тест 7.1: Спецификация с несколькими параметрами успешно загружается
TEST_F(UpsModelSpecTest, Load_MultipleParameters_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3.1
inputVoltage.normal = 200..259

inputFreq.oid = 1.2.3.2
inputFreq.normal = 400..600

outputStatus.oid = 1.2.3.3
outputStatus.normal = 2,3
)");
    expectLoadSuccess(ini, "TestUPS");

    const auto& params = m_spec.parameters();
    ASSERT_EQ(params.size(), 3u);

    // inputVoltage
    {
        const auto it = params.find("inputVoltage");
        ASSERT_NE(it, params.end());
        const ups::NormalValueSpec& normal = it->second.normal;
        EXPECT_TRUE(normal.isRange);
        EXPECT_EQ(normal.min, 200u);
        EXPECT_EQ(normal.max, 259u);
    }

    // inputFreq
    {
        const auto it = params.find("inputFreq");
        ASSERT_NE(it, params.end());
        const ups::NormalValueSpec& normal = it->second.normal;
        EXPECT_TRUE(normal.isRange);
        EXPECT_EQ(normal.min, 400u);
        EXPECT_EQ(normal.max, 600u);
    }

    // outputStatus
    {
        const auto it = params.find("outputStatus");
        ASSERT_NE(it, params.end());
        const ups::NormalValueSpec& normal = it->second.normal;
        EXPECT_FALSE(normal.isRange);
        ASSERT_EQ(normal.values.size(), 2u);
        EXPECT_EQ(normal.values[0], 2u);
        EXPECT_EQ(normal.values[1], 3u);
    }
}

// Тест 7.2: Повторяющееся имя параметра недопустимо
TEST_F(UpsModelSpecTest, Load_DuplicateParameterName_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3.1
inputVoltage.normal = 200..259

# Повтор параметра с тем же именем
inputVoltage.oid = 1.2.3.9
inputVoltage.normal = 210..240
)");
    expectLoadFailure(ini, "TestUPS", "duplicate");
}
#endif

#if (1)  // Проверка модели

// Тест 8.1: Спецификация без параметров недопустима
TEST_F(UpsModelSpecTest, Load_ModelWithoutParameters_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
)");
    expectLoadFailure(ini, "TestUPS", "no parameters");
}

// Тест 8.2: Спецификация с ровно одним параметром допустима
TEST_F(UpsModelSpecTest, Load_ModelWithSingleParameter_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

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

#if (1)  // Режим байпаса

// Тест 9.1: bypass — корректный enum
TEST_F(UpsModelSpecTest, Load_BypassEnumValid_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

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
TEST_F(UpsModelSpecTest, Load_BypassSingleValue_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

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
TEST_F(UpsModelSpecTest, Load_BypassInvalidNonNumeric_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

outputStatus.oid = 1.2.3.4
outputStatus.normal = 1
outputStatus.bypass = 6,abc
)");
    expectLoadFailure(ini, "TestUPS", "bypass");
}

// Тест 9.4: bypass отсутствует — это допустимо
TEST_F(UpsModelSpecTest, Load_WithoutBypass_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

outputStatus.oid = 1.2.3.4
outputStatus.normal = 1,2
)");
    expectLoadSuccess(ini, "TestUPS");

    const auto& params = m_spec.parameters();
    const auto it = params.find("outputStatus");
    ASSERT_NE(it, params.end());
    EXPECT_TRUE(it->second.bypass.empty());
}
#endif
