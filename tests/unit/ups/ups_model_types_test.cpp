/**
 * @file ups_model_types_test.cpp
 * @brief Unit-тесты функций ups_model_types.
 *
 * Проверяется:
 *  - корректность isFailureCause()
 *  - корректность paramToDescMap()
 *  - обработка неизвестных значений
 */

#include "ups_model_types.h"

#include <gtest/gtest.h>

#if (1)  // Проверка критичности отклонений

// Тест 1.1: OUTPUT_FAILURE является причиной аварии
TEST(UpsModelTypesTest, IsFailureCause_OutputFailure_ReturnsTrue) {
    EXPECT_TRUE(ups::isFailureCause(ups::UpsStateDesc::OUTPUT_FAILURE));
}

// Тест 1.2: Все alert-состояния не считаются причиной аварии
TEST(UpsModelTypesTest, IsFailureCause_Alerts_ReturnFalse) {
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::BATTERY_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::CHARGE_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::TEMP_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::FREQ_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::INPUT_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::BYPASS_ALERT));
}

// Тест 1.3: NONE не является причиной аварии
TEST(UpsModelTypesTest, IsFailureCause_None_ReturnsFalse) {
    EXPECT_FALSE(ups::isFailureCause(ups::UpsStateDesc::NONE));
}

// Тест 1.4: Неизвестное значение enum - false (default)
TEST(UpsModelTypesTest, IsFailureCause_UnknownEnum_ReturnsFalse) {
    const auto unknown = static_cast<ups::UpsStateDesc>(999);
    EXPECT_FALSE(ups::isFailureCause(unknown));
}
#endif

#if (1)  // Сопоставление параметров с отклонениями

// Тест 2.1: Корректное сопоставление параметров
TEST(UpsModelTypesTest, ParamToDescMap_KnownParams_ReturnExpectedDesc) {
    EXPECT_EQ(ups::paramToDescMap("batteryStatus"), ups::UpsStateDesc::BATTERY_ALERT);
    EXPECT_EQ(ups::paramToDescMap("chargeRemaining"), ups::UpsStateDesc::CHARGE_ALERT);
    EXPECT_EQ(ups::paramToDescMap("batteryTemp"), ups::UpsStateDesc::TEMP_ALERT);
    EXPECT_EQ(ups::paramToDescMap("inputFreq"), ups::UpsStateDesc::FREQ_ALERT);
    EXPECT_EQ(ups::paramToDescMap("inputVoltage"), ups::UpsStateDesc::INPUT_ALERT);
    EXPECT_EQ(ups::paramToDescMap("outputVoltage"), ups::UpsStateDesc::OUTPUT_FAILURE);
    EXPECT_EQ(ups::paramToDescMap("outputStatus"), ups::UpsStateDesc::BYPASS_ALERT);
}

// Тест 2.2: Неизвестный параметр возвращает NONE
TEST(UpsModelTypesTest, ParamToDescMap_UnknownParam_ReturnsNone) {
    EXPECT_EQ(ups::paramToDescMap("unknownParam"), ups::UpsStateDesc::NONE);
}

// Тест 2.3: Пустое имя параметра возвращает NONE
TEST(UpsModelTypesTest, ParamToDescMap_EmptyParam_ReturnsNone) {
    EXPECT_EQ(ups::paramToDescMap(""), ups::UpsStateDesc::NONE);
}
#endif
