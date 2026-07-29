/**
 * @file ups_model_detector_test.cpp
 * @brief Unit-тесты определения модели ИБП.
 *
 * Проверяется:
 *  - обработка ошибок чтения INI-файла
 *  - пропуск некорректных секций моделей
 *  - обработка ошибок и некорректных ответов SNMP
 *  - сопоставление полученного имени модели
 *  - успешное определение модели
 *
 * Используются временные INI-файлы, создаваемые прямо в тестах.
 */

#include "ups_model_detector.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "fake_snmp_client.h"
#include "temp_ini_file.h"

class UpsModelDetectorTest : public ::testing::Test {
protected:
    snmp::codec::SnmpValue makeStringValue(const std::string& value) {
        snmp::codec::SnmpValue result;
        result.type = snmp::codec::SnmpValue::Type::String;
        result.strValue = value;
        return result;
    }

    snmp::codec::SnmpValue makeIntValue(uint32_t value) {
        snmp::codec::SnmpValue result;
        result.type = snmp::codec::SnmpValue::Type::Integer;
        result.intValue = value;
        return result;
    }

    test::FakeSnmpClient m_snmp;
    std::string m_model;
    snmp::ErrorMessage m_error;
    test::TempIniFileStorage m_tempIniFiles;
};

#if (1)  // Ошибки чтения INI-файла

// Тест 1.1: Определение модели невозможно, если INI-файл не существует
TEST_F(UpsModelDetectorTest, Detect_FileNotFound_ReturnsError) {
    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, "no_such_file.ini", m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_FALSE(m_error.empty());
}
#endif

#if (1)  // Некорректные секции моделей

// Тест 2.1: Некорректная секция модели пропускается
TEST_F(UpsModelDetectorTest, Detect_InvalidSection_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
# отсутствует modelName
param1.oid = 1.2.3
)");

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [TEST]: invalid specification: modelName missing");
}
#endif

#if (1)  // Метаданные модели

// Тест 3.1: Секция с пустым именем модели пропускается
TEST_F(UpsModelDetectorTest, Detect_ModelNameEmpty_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName =
modelName.oid = 1.2.3
)");

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [TEST]: invalid specification: modelName missing");
}

// Тест 3.2: Секция без OID имени модели пропускается
TEST_F(UpsModelDetectorTest, Detect_ModelNameOidMissing_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
)");

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [TEST]: invalid specification: modelName.oid missing");
}
#endif

#if (1)  // Ошибки SNMP-запросов

// Тест 4.1: Секция пропускается, если SNMP GET-запрос завершился ошибкой
TEST_F(UpsModelDetectorTest, Detect_SnmpGetFails_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    m_snmp.set(snmp::Oid("1.2.3"), { false, {}, "SNMP request failed" });

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [TEST]: SNMP request failed");
}
#endif

#if (1)  // Проверка SNMP-ответов

// Тест 5.1: Секция пропускается, если SNMP-ответ не содержит строку
TEST_F(UpsModelDetectorTest, Detect_SnmpValueNotString_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    m_snmp.set(snmp::Oid("1.2.3"), { true, makeIntValue(42), {} });

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [TEST]: SNMP response is not a string");
}
#endif

#if (1)  // Сопоставление имени модели

// Тест 6.1: Определение модели невозможно, если имя из SNMP-ответа не совпало
TEST_F(UpsModelDetectorTest, Detect_ModelNameMismatch_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    m_snmp.set(snmp::Oid("1.2.3"), { true, makeStringValue("OTHER_UPS"), {} });

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [TEST]: model name does not match");
}
#endif

#if (1)  // Успешное определение модели

// Тест 7.1: Для совпавшего имени возвращается идентификатор секции модели
TEST_F(UpsModelDetectorTest, Detect_ModelNameMatches_ReturnsSectionId) {
    const std::string ini = m_tempIniFiles.write(R"(
[APC]
modelName = Smart-UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.4
inputVoltage.normal = 200..240
)");
    m_snmp.set(snmp::Oid("1.2.3"), { true, makeStringValue("APC Smart-UPS 1500"), {} });

    EXPECT_TRUE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_error.empty());
    EXPECT_EQ(m_model, "APC");
}
#endif

#if (1)  // Отсутствие подходящей модели

// Тест 8.1: Возвращается ошибка, если ни одна секция модели не подошла
TEST_F(UpsModelDetectorTest, Detect_NoMatchingSection_ReturnsError) {
    const std::string ini = m_tempIniFiles.write(R"(
[A]
modelName = A_UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.1
inputVoltage.normal = 200..240

[B]
modelName = B_UPS
modelName.oid = 1.2.4

inputVoltage.oid = 1.2.4.1
inputVoltage.normal = 200..240
)");
    m_snmp.set(snmp::Oid("1.2.3"), { true, makeStringValue("X"), {} });
    m_snmp.set(snmp::Oid("1.2.4"), { true, makeStringValue("Y"), {} });

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [A]: model name does not match\n"
              "section [B]: model name does not match");
}

// Тест 8.2: В ошибку определения модели включаются причины по всем секциям
TEST_F(UpsModelDetectorTest, Detect_SectionsRejectedForDifferentReasons_ReturnsAllErrors) {
    const std::string ini = m_tempIniFiles.write(R"(
[A]
modelName = A_UPS
modelName.oid = 1.2.3

inputVoltage.oid = 1.2.3.1
inputVoltage.normal = 200..240

[B]
modelName = B_UPS
modelName.oid = 1.2.4

inputVoltage.oid = 1.2.4.1
inputVoltage.normal = 200..240

[C]
modelName = C_UPS
modelName.oid = 1.2.5

inputVoltage.oid = 1.2.5.1
inputVoltage.normal = 200..240
)");
    m_snmp.set(snmp::Oid("1.2.3"), { false, {}, "SNMP response timeout" });
    m_snmp.set(snmp::Oid("1.2.4"), { true, makeIntValue(42), {} });
    m_snmp.set(snmp::Oid("1.2.5"), { true, makeStringValue("OTHER_UPS"), {} });

    EXPECT_FALSE(ups::UpsModelDetector::detect(m_snmp, ini, m_model, m_error));
    EXPECT_TRUE(m_model.empty());
    EXPECT_EQ(m_error,
              "UPS model could not be detected\n"
              "section [A]: SNMP response timeout\n"
              "section [B]: SNMP response is not a string\n"
              "section [C]: model name does not match");
}
#endif
