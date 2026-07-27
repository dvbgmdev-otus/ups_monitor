/**
 * @file ups_model_spec_test.cpp
 * @brief Unit-тесты загрузки и структуры спецификации модели ИБП.
 *
 * Проверяется:
 *  - обработка отсутствующего файла и секции
 *  - проверка обязательных полей модели и параметров
 *  - загрузка нескольких параметров
 *
 * Используются временные INI-файлы, создаваемые прямо в тестах.
 */

#include "ups_model_spec_test_base.h"

#include <string>

class UpsModelSpecTest : public UpsModelSpecTestBase {};

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

// Тест 3.2: Метаданные модели доступны после успешной загрузки
TEST_F(UpsModelSpecTest, Load_ValidMetadata_ReturnsModelNameAndOid) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    expectLoadSuccess(ini, "TestUPS");

    EXPECT_EQ(m_spec.modelName(), "Test UPS");
    EXPECT_EQ(m_spec.modelNameOid(), "1.2.3");
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
modelName.oid = 1.2.3
inputVoltage.oid = 1.2.3.4
)");
    expectLoadFailure(ini, "TestUPS", "normal");
}
#endif

#if (1)  // Несколько параметров

// Тест 7.1: Спецификация с несколькими параметрами успешно загружается
TEST_F(UpsModelSpecTest, Load_MultipleParameters_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

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
modelName.oid = 1.2.3
)");
    expectLoadFailure(ini, "TestUPS", "no parameters");
}

// Тест 8.2: Спецификация с ровно одним параметром допустима
TEST_F(UpsModelSpecTest, Load_ModelWithSingleParameter_Succeeds) {
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
