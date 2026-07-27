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

#if (1)  // Часть 1 — Предварительные условия

// Тест 1.1: Загрузка спецификации невозможна, если INI-файл не существует
TEST_F(UpsModelSpecTest, Load_FileNotFound_ReturnsError) {
    expectLoadFailure("non_existing_file.ini", "APC", "file not found");
}
#endif

#if (1)  // Часть 2 — Проверка секции модели

// Тест 2.1: Загрузка невозможна, если секция модели отсутствует
TEST_F(UpsModelSpecTest, Load_SectionNotFound_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3
inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "NON_EXISTING_MODEL", "section not found");
}
#endif

#if (1)  // Часть 3 — Метаданные модели

// Тест 3.1: Загрузка невозможна, если в секции отсутствует modelName
TEST_F(UpsModelSpecTest, Load_ModelNameMissing_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3
inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "TestUPS", "modelName missing");
}

// Тест 3.2: Загрузка невозможна, если отсутствует modelName.oid
TEST_F(UpsModelSpecTest, Load_ModelNameOidMissing_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "TestUPS", "modelName.oid missing");
}

// Тест 3.3: Метаданные модели доступны после успешной загрузки
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

#if (1)  // Часть 4 — Спецификация параметра

// Тест 4.1: Загрузка невозможна, если у параметра отсутствует .oid
TEST_F(UpsModelSpecTest, Load_ParamWithoutOid_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.normal = 200..240
)");
    expectLoadFailure(ini, "TestUPS", "parameter without oid: inputVoltage");
}
#endif

#if (1)  // Часть 5 — Критерии нормального состояния

// Тест 5.1: Загрузка невозможна, если у параметра отсутствуют normal и bypass
TEST_F(UpsModelSpecTest, Load_ParamWithoutNormalOrBypass_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
)");
    expectLoadFailure(ini, "TestUPS", "parameter without normal or bypass: inputVoltage");
}

// Тест 5.2: Параметр с normal допустим без bypass
TEST_F(UpsModelSpecTest, Load_ParamWithNormalWithoutBypass_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    expectLoadSuccess(ini, "TestUPS");
}

// Тест 5.3: Параметр с bypass допустим без normal
TEST_F(UpsModelSpecTest, Load_ParamWithBypassWithoutNormal_Succeeds) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

outputStatus.oid = 1.2.3.4
outputStatus.bypass = 6
)");
    expectLoadSuccess(ini, "TestUPS");
}
#endif

#if (1)  // Часть 6 — Набор параметров

// Тест 6.1: Спецификация без параметров недопустима
TEST_F(UpsModelSpecTest, Load_ModelWithoutParameters_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3
)");
    expectLoadFailure(ini, "TestUPS", "no parameters defined");
}

// Тест 6.2: Спецификация с ровно одним параметром допустима
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
    EXPECT_NE(params.find("inputVoltage"), params.end());
}

// Тест 6.3: Спецификация с несколькими параметрами успешно загружается
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
    EXPECT_NE(params.find("inputVoltage"), params.end());
    EXPECT_NE(params.find("inputFreq"), params.end());
    EXPECT_NE(params.find("outputStatus"), params.end());
}

// Тест 6.4: Повторное объявление OID параметра недопустимо
TEST_F(UpsModelSpecTest, Load_DuplicateParameterOid_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TestUPS]
modelName = Test UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.1
inputVoltage.normal = 200..259

# Повторное объявление OID того же параметра
inputVoltage.oid = 1.2.3.9
inputVoltage.normal = 210..240
)");
    expectLoadFailure(ini, "TestUPS", "duplicate parameter: inputVoltage");
}
#endif
