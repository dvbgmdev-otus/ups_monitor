/**
 * @file ups_param_checker_test.cpp
 * @brief Unit-тесты для UpsParamChecker.
 */
#include <gtest/gtest.h>

#include "ups_param_checker.h"
#include "ups_model_types.h"

namespace {

using snmp::codec::SnmpValue;

class UpsParamCheckerTest : public ::testing::Test {
protected:
    uint32_t m_descr{ 0 };

    void SetUp() override { m_descr = 0; }

    static SnmpValue makeIntValue(int value) {
        SnmpValue result;
        result.type = SnmpValue::Type::Integer;
        result.intValue = value;
        return result;
    }

    static SnmpValue makeNullValue() {
        SnmpValue result;
        result.type = SnmpValue::Type::Null;
        return result;
    }

    static SnmpValue makeStringValue(const std::string& value) {
        SnmpValue result;
        result.type = SnmpValue::Type::String;
        result.strValue = value;
        return result;
    }
};

#if (1)  // Проверка нормальных значений

// Тест 1.1: Значение в диапазоне считается допустимым
TEST_F(UpsParamCheckerTest, NormalRange_ValueInRange_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;

    const bool ok = ups::UpsParamChecker::check(
        spec, makeIntValue(230), ups::UpsDeviationFlags::INPUT_ALERT, m_descr);

    EXPECT_TRUE(ok);
    EXPECT_EQ(m_descr, 0u);
}

// Тест 1.2: Значение из перечисления считается допустимым
TEST_F(UpsParamCheckerTest, NormalEnum_ValueInEnum_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "batteryStatus";
    spec.normal.values = { 2, 3 };

    const bool ok = ups::UpsParamChecker::check(
        spec, makeIntValue(2), ups::UpsDeviationFlags::BATTERY_ALERT, m_descr);

    EXPECT_TRUE(ok);
    EXPECT_EQ(m_descr, 0u);
}

// Тест 1.3: Работа не на байпасе не устанавливает флаг отклонения
TEST_F(UpsParamCheckerTest, BypassValue_NotInBypass_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "outputStatus";
    spec.bypass = { 6, 9, 10 };

    const bool ok = ups::UpsParamChecker::check(
        spec, makeIntValue(0), ups::UpsDeviationFlags::BYPASS_ALERT, m_descr);

    EXPECT_TRUE(ok);
    EXPECT_EQ(m_descr, 0u);
}

#endif

#if (1)  // Проверка выхода за допустимые значения

// Тест 2.1: Выход за диапазон устанавливает предупреждающий флаг
TEST_F(UpsParamCheckerTest, NormalRange_ValueOutOfRange_SetsAlert) {
    ups::UpsParamSpec spec;
    spec.name = "batteryTemp";
    spec.normal.isRange = true;
    spec.normal.min = 0;
    spec.normal.max = 50;

    const bool ok = ups::UpsParamChecker::check(
        spec, makeIntValue(80), ups::UpsDeviationFlags::TEMP_ALERT, m_descr);

    EXPECT_TRUE(ok);
    EXPECT_NE(m_descr & static_cast<uint32_t>(ups::UpsDeviationFlags::TEMP_ALERT), 0u);
}

// Тест 2.2: Выходное напряжение вне диапазона устанавливает аварийный флаг
TEST_F(UpsParamCheckerTest, OutputVoltage_OutOfRange_SetsFailure) {
    ups::UpsParamSpec spec;
    spec.name = "outputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;

    const bool ok = ups::UpsParamChecker::check(
        spec, makeIntValue(180), ups::UpsDeviationFlags::OUTPUT_FAILURE, m_descr);

    EXPECT_TRUE(ok);
    EXPECT_NE(m_descr & static_cast<uint32_t>(ups::UpsDeviationFlags::OUTPUT_FAILURE), 0u);
}

#endif

#if (1)  // Проверка режима байпаса

// Тест 3.1: Значение байпаса устанавливает соответствующий флаг
TEST_F(UpsParamCheckerTest, BypassValue_SetsBypassAlert) {
    ups::UpsParamSpec spec;
    spec.name = "outputStatus";
    spec.bypass = { 6, 9, 10 };

    const bool ok = ups::UpsParamChecker::check(
        spec, makeIntValue(6), ups::UpsDeviationFlags::BYPASS_ALERT, m_descr);

    EXPECT_TRUE(ok);
    EXPECT_NE(m_descr & static_cast<uint32_t>(ups::UpsDeviationFlags::BYPASS_ALERT), 0u);
}

#endif

#if (1)  // Проверка отсутствующих и некорректных данных

// Тест 4.1: Null-значение считается отсутствием данных
TEST_F(UpsParamCheckerTest, NullValue_ReturnsFalse) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;

    const bool ok = ups::UpsParamChecker::check(
        spec, makeNullValue(), ups::UpsDeviationFlags::INPUT_ALERT, m_descr);

    EXPECT_FALSE(ok);
    EXPECT_EQ(m_descr, 0u);
}

// Тест 4.2: Строковое значение считается некорректным
TEST_F(UpsParamCheckerTest, StringValue_ReturnsFalse) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;

    const bool ok = ups::UpsParamChecker::check(
        spec, makeStringValue("invalid"), ups::UpsDeviationFlags::INPUT_ALERT, m_descr);

    EXPECT_FALSE(ok);
    EXPECT_EQ(m_descr, 0u);
}

#endif

}  // namespace
