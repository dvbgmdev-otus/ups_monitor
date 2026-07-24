/**
 * @file ups_model_types_test.cpp
 * @brief Unit-тесты функций ups_model_types.
 *
 * Проверяется:
 *  - корректность isFailureCause()
 *  - корректность toDeviationFlag()
 *  - побитовые операции с UpsDeviationFlags
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
    EXPECT_TRUE(ups::isFailureCause(ups::UpsDeviationFlags::OUTPUT_FAILURE));
}

// Тест 1.2: Все alert-состояния не считаются причиной аварии
TEST(UpsModelTypesTest, IsFailureCause_Alerts_ReturnFalse) {
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::BATTERY_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::CHARGE_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::TEMP_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::FREQ_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::INPUT_ALERT));
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::BYPASS_ALERT));
}

// Тест 1.3: NONE не является причиной аварии
TEST(UpsModelTypesTest, IsFailureCause_None_ReturnsFalse) {
    EXPECT_FALSE(ups::isFailureCause(ups::UpsDeviationFlags::NONE));
}

// Тест 1.4: Неизвестное значение enum - false (default)
TEST(UpsModelTypesTest, IsFailureCause_UnknownEnum_ReturnsFalse) {
    const auto unknown = static_cast<ups::UpsDeviationFlags>(999);
    EXPECT_FALSE(ups::isFailureCause(unknown));
}
#endif

#if (1)  // Сопоставление параметров с отклонениями

// Тест 2.1: Корректное сопоставление параметров
TEST(UpsModelTypesTest, ToDeviationFlag_KnownParams_ReturnsExpectedFlag) {
    EXPECT_EQ(ups::toDeviationFlag("batteryStatus"), ups::UpsDeviationFlags::BATTERY_ALERT);
    EXPECT_EQ(ups::toDeviationFlag("chargeRemaining"), ups::UpsDeviationFlags::CHARGE_ALERT);
    EXPECT_EQ(ups::toDeviationFlag("batteryTemp"), ups::UpsDeviationFlags::TEMP_ALERT);
    EXPECT_EQ(ups::toDeviationFlag("inputFreq"), ups::UpsDeviationFlags::FREQ_ALERT);
    EXPECT_EQ(ups::toDeviationFlag("inputVoltage"), ups::UpsDeviationFlags::INPUT_ALERT);
    EXPECT_EQ(ups::toDeviationFlag("outputVoltage"), ups::UpsDeviationFlags::OUTPUT_FAILURE);
    EXPECT_EQ(ups::toDeviationFlag("outputStatus"), ups::UpsDeviationFlags::BYPASS_ALERT);
}

// Тест 2.2: Неизвестный параметр возвращает NONE
TEST(UpsModelTypesTest, ToDeviationFlag_UnknownParam_ReturnsNone) {
    EXPECT_EQ(ups::toDeviationFlag("unknownParam"), ups::UpsDeviationFlags::NONE);
}

// Тест 2.3: Пустое имя параметра возвращает NONE
TEST(UpsModelTypesTest, ToDeviationFlag_EmptyParam_ReturnsNone) {
    EXPECT_EQ(ups::toDeviationFlag(""), ups::UpsDeviationFlags::NONE);
}
#endif

#if (1)  // Побитовые операции с отклонениями

// Тест 3.1: operator| объединяет несколько причин отклонений
TEST(UpsModelTypesTest, BitwiseOr_TwoDeviations_ReturnsCombinedMask) {
    const ups::UpsDeviationFlags result =
        ups::UpsDeviationFlags::CHARGE_ALERT | ups::UpsDeviationFlags::BYPASS_ALERT;

    EXPECT_EQ(static_cast<uint32_t>(result), 0x00000042u);
}

// Тест 3.2: operator|= добавляет причину в существующую маску
TEST(UpsModelTypesTest, BitwiseOrAssign_Deviation_AddsFlagToMask) {
    ups::UpsDeviationFlags result = ups::UpsDeviationFlags::BATTERY_ALERT;

    result |= ups::UpsDeviationFlags::OUTPUT_FAILURE;

    EXPECT_EQ(static_cast<uint32_t>(result), 0x00000021u);
}

// Тест 3.3: operator& возвращает общую часть двух масок
TEST(UpsModelTypesTest, BitwiseAnd_TwoMasks_ReturnsCommonFlags) {
    const ups::UpsDeviationFlags lhs =
        ups::UpsDeviationFlags::BATTERY_ALERT | ups::UpsDeviationFlags::OUTPUT_FAILURE;
    const ups::UpsDeviationFlags rhs =
        ups::UpsDeviationFlags::OUTPUT_FAILURE | ups::UpsDeviationFlags::BYPASS_ALERT;

    EXPECT_EQ(lhs & rhs, ups::UpsDeviationFlags::OUTPUT_FAILURE);
}

// Тест 3.4: hasFlag обнаруживает установленную причину
TEST(UpsModelTypesTest, HasFlag_FlagIsSet_ReturnsTrue) {
    const ups::UpsDeviationFlags value =
        ups::UpsDeviationFlags::CHARGE_ALERT | ups::UpsDeviationFlags::BYPASS_ALERT;

    EXPECT_TRUE(ups::hasFlag(value, ups::UpsDeviationFlags::BYPASS_ALERT));
}

// Тест 3.5: hasFlag возвращает false для отсутствующей причины
TEST(UpsModelTypesTest, HasFlag_FlagIsNotSet_ReturnsFalse) {
    const ups::UpsDeviationFlags value =
        ups::UpsDeviationFlags::CHARGE_ALERT | ups::UpsDeviationFlags::BYPASS_ALERT;

    EXPECT_FALSE(ups::hasFlag(value, ups::UpsDeviationFlags::OUTPUT_FAILURE));
}
#endif

#if (1)  // Значения состояния по умолчанию

// Тест 4.1: Новое состояние не содержит информации и отклонений
TEST(UpsModelTypesTest, UpsState_DefaultValues_ReturnsNoInfoWithoutDeviations) {
    const ups::UpsState state;

    EXPECT_EQ(state.status, ups::UpsStatus::NO_INFO);
    EXPECT_EQ(state.deviations, ups::UpsDeviationFlags::NONE);
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
