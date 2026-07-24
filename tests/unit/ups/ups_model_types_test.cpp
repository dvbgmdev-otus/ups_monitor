/**
 * @file ups_model_types_test.cpp
 * @brief Unit-тесты функций ups_model_types.
 *
 * Проверяется:
 *  - корректность isFailureCause()
 *  - корректность paramToDescMap()
 *  - побитовые операции с UpsStateDesc
 *  - значения по умолчанию UpsState
 *  - строковое представление UpsStatus
 *  - обработка неизвестных значений
 */

#include "ups_model_types.h"

#include <gtest/gtest.h>

#include <cstdint>

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

#if (1)  // Побитовые операции с отклонениями

// Тест 3.1: operator| объединяет несколько причин отклонений
TEST(UpsModelTypesTest, BitwiseOr_TwoDeviations_ReturnsCombinedMask) {
    const ups::UpsStateDesc result =
        ups::UpsStateDesc::CHARGE_ALERT | ups::UpsStateDesc::BYPASS_ALERT;

    EXPECT_EQ(static_cast<uint32_t>(result), 0x00000042u);
}

// Тест 3.2: operator|= добавляет причину в существующую маску
TEST(UpsModelTypesTest, BitwiseOrAssign_Deviation_AddsFlagToMask) {
    ups::UpsStateDesc result = ups::UpsStateDesc::BATTERY_ALERT;

    result |= ups::UpsStateDesc::OUTPUT_FAILURE;

    EXPECT_EQ(static_cast<uint32_t>(result), 0x00000021u);
}

// Тест 3.3: operator& возвращает общую часть двух масок
TEST(UpsModelTypesTest, BitwiseAnd_TwoMasks_ReturnsCommonFlags) {
    const ups::UpsStateDesc lhs =
        ups::UpsStateDesc::BATTERY_ALERT | ups::UpsStateDesc::OUTPUT_FAILURE;
    const ups::UpsStateDesc rhs =
        ups::UpsStateDesc::OUTPUT_FAILURE | ups::UpsStateDesc::BYPASS_ALERT;

    EXPECT_EQ(lhs & rhs, ups::UpsStateDesc::OUTPUT_FAILURE);
}

// Тест 3.4: hasFlag обнаруживает установленную причину
TEST(UpsModelTypesTest, HasFlag_FlagIsSet_ReturnsTrue) {
    const ups::UpsStateDesc value =
        ups::UpsStateDesc::CHARGE_ALERT | ups::UpsStateDesc::BYPASS_ALERT;

    EXPECT_TRUE(ups::hasFlag(value, ups::UpsStateDesc::BYPASS_ALERT));
}

// Тест 3.5: hasFlag возвращает false для отсутствующей причины
TEST(UpsModelTypesTest, HasFlag_FlagIsNotSet_ReturnsFalse) {
    const ups::UpsStateDesc value =
        ups::UpsStateDesc::CHARGE_ALERT | ups::UpsStateDesc::BYPASS_ALERT;

    EXPECT_FALSE(ups::hasFlag(value, ups::UpsStateDesc::OUTPUT_FAILURE));
}
#endif

#if (1)  // Значения состояния по умолчанию

// Тест 4.1: Новое состояние не содержит информации и отклонений
TEST(UpsModelTypesTest, UpsState_DefaultValues_ReturnsNoInfoWithoutDeviations) {
    const ups::UpsState state;

    EXPECT_EQ(state.status, ups::UpsStatus::NO_INFO);
    EXPECT_EQ(state.descr, ups::UpsStateDesc::NONE);
}
#endif

#if (1)  // Строковое представление состояния

// Тест 5.1: Все состояния преобразуются в строки внешнего API
TEST(UpsModelTypesTest, ToString_KnownStatuses_ReturnsApiValues) {
    EXPECT_STREQ(ups::toString(ups::UpsStatus::OK), "OK");
    EXPECT_STREQ(ups::toString(ups::UpsStatus::WARNING), "WARNING");
    EXPECT_STREQ(ups::toString(ups::UpsStatus::FAILURE), "FAILURE");
    EXPECT_STREQ(ups::toString(ups::UpsStatus::NO_INFO), "NO_INFO");
}

// Тест 5.2: Неизвестное состояние преобразуется в UNKNOWN
TEST(UpsModelTypesTest, ToString_UnknownStatus_ReturnsUnknown) {
    const auto unknown = static_cast<ups::UpsStatus>(999);

    EXPECT_STREQ(ups::toString(unknown), "UNKNOWN");
}
#endif
