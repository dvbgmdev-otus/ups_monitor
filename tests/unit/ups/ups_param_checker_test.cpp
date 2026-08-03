/**
 * @file ups_param_checker_test.cpp
 * @brief Unit-тесты для UpsParamChecker.
 *
 * Проверяется:
 *  - обработка допустимых значений
 *  - установка флагов при отклонении от нормы
 *  - определение режима байпаса
 *  - обработка отсутствующих и некорректных данных
 *  - накопление отклонений
 *  - совместная проверка normal и bypass
 *  - обработка отрицательных значений
 */

#include "ups_param_checker.h"

#include <gtest/gtest.h>

#include <string>

#include "ups_model_types.h"

using snmp::codec::SnmpValue;

class UpsParamCheckerTest : public ::testing::Test {
protected:
    ups::UpsDeviationFlags m_deviations{ ups::UpsDeviationFlags::NONE };

    void SetUp() override { m_deviations = ups::UpsDeviationFlags::NONE; }

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

#if (1)  // Часть 1 — Общая проверка normal

// Тест 1.1: Значение в диапазоне считается допустимым
TEST_F(UpsParamCheckerTest, NormalRange_ValueInRange_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(230), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

// Тест 1.2: Значение из перечисления считается допустимым
TEST_F(UpsParamCheckerTest, NormalEnum_ValueInEnum_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "batteryStatus";
    spec.normal.values = { 2, 3 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(2), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

// Тест 1.3: Работа не на байпасе не устанавливает флаг отклонения
TEST_F(UpsParamCheckerTest, BypassValue_NotInBypass_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "outputStatus";
    spec.bypass = { 6, 9, 10 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(0), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

// Тест 1.4: Значение на нижней границе диапазона считается допустимым
TEST_F(UpsParamCheckerTest, NormalRange_ValueEqualsMin_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(200), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

// Тест 1.5: Значение на верхней границе диапазона считается допустимым
TEST_F(UpsParamCheckerTest, NormalRange_ValueEqualsMax_Ok) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(259), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

#endif

#if (1)  // Часть 2 — Проверка выхода за допустимые значения

// Тест 2.1: Выход за диапазон устанавливает предупреждающий флаг
TEST_F(UpsParamCheckerTest, NormalRange_ValueOutOfRange_SetsAlert) {
    ups::UpsParamSpec spec;
    spec.name = "batteryTemp";
    spec.normal.isRange = true;
    spec.normal.min = 0;
    spec.normal.max = 50;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(80), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::TEMP_ALERT));
}

// Тест 2.2: Выходное напряжение вне диапазона устанавливает аварийный флаг
TEST_F(UpsParamCheckerTest, OutputVoltage_OutOfRange_SetsFailure) {
    ups::UpsParamSpec spec;
    spec.name = "outputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(180), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::OUTPUT_FAILURE));
}

// Тест 2.3: Значение вне перечисления устанавливает предупреждающий флаг
TEST_F(UpsParamCheckerTest, NormalEnum_ValueNotInEnum_SetsAlert) {
    ups::UpsParamSpec spec;
    spec.name = "batteryStatus";
    spec.normal.values = { 2, 3 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(1), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::BATTERY_ALERT);
}

#endif

#if (1)  // Часть 3 — Проверка режима байпаса

// Тест 3.1: Значение байпаса устанавливает соответствующий флаг
TEST_F(UpsParamCheckerTest, BypassValue_SetsBypassAlert) {
    ups::UpsParamSpec spec;
    spec.name = "outputStatus";
    spec.bypass = { 6, 9, 10 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(6), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::BYPASS_ALERT));
}

#endif

#if (1)  // Часть 4 — Проверка отсутствующих и некорректных данных

// Тест 4.1: Null-значение считается отсутствием данных
TEST_F(UpsParamCheckerTest, NullValue_ReturnsFalse) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    const bool ok = ups::UpsParamChecker::check(spec, makeNullValue(), m_deviations);
    EXPECT_FALSE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

// Тест 4.2: Строковое значение считается некорректным
TEST_F(UpsParamCheckerTest, StringValue_ReturnsFalse) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    const bool ok = ups::UpsParamChecker::check(spec, makeStringValue("invalid"), m_deviations);
    EXPECT_FALSE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

#endif

#if (1)  // Часть 5 — Проверка накопления отклонений

// Тест 5.1: Новое отклонение добавляется к ранее накопленному флагу
TEST_F(UpsParamCheckerTest, Deviations_PreviousFlagPresent_AddsNewFlag) {
    ups::UpsParamSpec spec;
    spec.name = "batteryTemp";
    spec.normal.isRange = true;
    spec.normal.min = 0;
    spec.normal.max = 50;
    m_deviations = ups::UpsDeviationFlags::BATTERY_ALERT;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(80), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::BATTERY_ALERT));
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::TEMP_ALERT));
}

// Тест 5.2: Нормальное значение не очищает ранее накопленный флаг
TEST_F(UpsParamCheckerTest, Deviations_NormalValue_PreservesPreviousFlag) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    m_deviations = ups::UpsDeviationFlags::BATTERY_ALERT;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(230), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::BATTERY_ALERT);
}

// Тест 5.3: Некорректный тип данных не изменяет ранее накопленный флаг
TEST_F(UpsParamCheckerTest, Deviations_InvalidValue_PreservesPreviousFlag) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    m_deviations = ups::UpsDeviationFlags::BATTERY_ALERT;
    const bool ok = ups::UpsParamChecker::check(spec, makeStringValue("invalid"), m_deviations);
    EXPECT_FALSE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::BATTERY_ALERT);
}

#endif

#if (1)  // Часть 6 — Совместная проверка normal и bypass

// Тест 6.1: Значение в норме, байпас включён — устанавливается только флаг байпаса
TEST_F(UpsParamCheckerTest, NormalAndBypass_ValueMatchesBoth_SetsBypassAlert) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    spec.bypass = { 230 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(230), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::BYPASS_ALERT);
}

// Тест 6.2: Значение вне нормы, байпас выключен — устанавливается только флаг параметра
TEST_F(UpsParamCheckerTest, NormalAndBypass_ValueMatchesNeither_SetsParamDeviation) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    spec.bypass = { 230 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(180), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::INPUT_ALERT);
}

// Тест 6.3: Значение вне нормы, байпас включён — устанавливаются оба флага
TEST_F(UpsParamCheckerTest, NormalAndBypass_BypassOutsideNormal_SetsBothDeviations) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    spec.bypass = { 180 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(180), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::BYPASS_ALERT));
    EXPECT_TRUE(ups::hasFlag(m_deviations, ups::UpsDeviationFlags::INPUT_ALERT));
}

// Тест 6.4: Значение в норме, байпас выключен — флаги не устанавливаются
TEST_F(UpsParamCheckerTest, NormalAndBypass_NormalOutsideBypass_NoDeviations) {
    ups::UpsParamSpec spec;
    spec.name = "inputVoltage";
    spec.normal.isRange = true;
    spec.normal.min = 200;
    spec.normal.max = 259;
    spec.bypass = { 180 };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(230), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

#endif

#if (1)  // Часть 7 — Проверка отрицательных значений

// Тест 7.1: Отрицательное значение не соответствует normal
TEST_F(UpsParamCheckerTest, NegativeValue_NormalPresent_SetsParamDeviation) {
    ups::UpsParamSpec spec;
    spec.name = "batteryTemp";
    spec.normal.isRange = true;
    spec.normal.min = 0;
    spec.normal.max = 50;
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(-1), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::TEMP_ALERT);
}

// Тест 7.2: Отрицательное значение не включает байпас
TEST_F(UpsParamCheckerTest, NegativeValue_BypassOnly_NoDeviations) {
    ups::UpsParamSpec spec;
    spec.name = "outputStatus";
    spec.bypass = { 4294967295u };
    const bool ok = ups::UpsParamChecker::check(spec, makeIntValue(-1), m_deviations);
    EXPECT_TRUE(ok);
    EXPECT_EQ(m_deviations, ups::UpsDeviationFlags::NONE);
}

#endif
