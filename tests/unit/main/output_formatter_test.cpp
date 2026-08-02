/**
 * @file output_formatter_test.cpp
 * @brief Unit-тесты форматирования пользовательского вывода приложения.
 */

#include "output_formatter.h"

#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
#include <string>

#include "ups_model_types.h"

namespace {

output::Timestamp makeLocalTimestamp(
    int year, int month, int day, int hour, int minute, int second) {
    std::tm localTime{};
    localTime.tm_year = year - 1900;
    localTime.tm_mon = month - 1;
    localTime.tm_mday = day;
    localTime.tm_hour = hour;
    localTime.tm_min = minute;
    localTime.tm_sec = second;
    localTime.tm_isdst = -1;
    return std::chrono::system_clock::from_time_t(std::mktime(&localTime));
}

class OutputFormatterTest : public ::testing::Test {
protected:
    const output::Timestamp m_timestamp = makeLocalTimestamp(2026, 7, 15, 14, 25, 43);
};

}  // namespace

#if (1)  // Part 1 — Сообщение об обнаруженной модели

// Test 1.1: Название модели выводится в установленном формате
TEST_F(OutputFormatterTest, FormatDetectedModel_ValidModel_ReturnsExactMessage) {
    EXPECT_EQ(output::formatDetectedModel("Smart-UPS RT 2000 XL", m_timestamp),
              "2026-07-15 14:25:43 [UPS] detected model=\"Smart-UPS RT 2000 XL\"");
}

#endif

#if (1)  // Part 2 — Сообщение о состоянии

// Test 2.1: Нормальное состояние выводится с нулевой восьмизначной маской
TEST_F(OutputFormatterTest, FormatState_OkWithoutDeviations_ReturnsExactMessage) {
    const ups::UpsState state{ ups::UpsStatus::OK, ups::UpsDeviationFlags::NONE };
    EXPECT_EQ(output::formatState(state, m_timestamp),
              "2026-07-15 14:25:43 [UPS] status=OK deviations=0x00000000");
}

// Test 2.2: Предупреждение выводится с объединённой маской отклонений
TEST_F(OutputFormatterTest, FormatState_WarningWithDeviations_ReturnsExactMessage) {
    const ups::UpsState state{ ups::UpsStatus::WARNING,
                               ups::UpsDeviationFlags::CHARGE_ALERT |
                                   ups::UpsDeviationFlags::BYPASS_ALERT };
    EXPECT_EQ(output::formatState(state, m_timestamp),
              "2026-07-15 14:25:43 [UPS] status=WARNING deviations=0x00000042");
}

// Test 2.3: Аварийное состояние выводится с восьмизначной маской
TEST_F(OutputFormatterTest, FormatState_FailureWithDeviation_ReturnsExactMessage) {
    const ups::UpsState state{ ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE };
    EXPECT_EQ(output::formatState(state, m_timestamp),
              "2026-07-15 14:25:43 [UPS] status=FAILURE deviations=0x00000020");
}

// Test 2.4: Отсутствие информации выводится как самостоятельное состояние
TEST_F(OutputFormatterTest, FormatState_NoInfo_ReturnsExactMessage) {
    const ups::UpsState state{ ups::UpsStatus::NO_INFO, ups::UpsDeviationFlags::NONE };
    EXPECT_EQ(output::formatState(state, m_timestamp),
              "2026-07-15 14:25:43 [UPS] status=NO_INFO deviations=0x00000000");
}

// Test 2.5: Шестнадцатеричные цифры маски выводятся в верхнем регистре
TEST_F(OutputFormatterTest, FormatState_MaskContainsHexLetters_ReturnsUppercaseDigits) {
    const ups::UpsDeviationFlags deviations =
        ups::UpsDeviationFlags::CHARGE_ALERT | ups::UpsDeviationFlags::FREQ_ALERT |
        ups::UpsDeviationFlags::OUTPUT_FAILURE | ups::UpsDeviationFlags::BYPASS_ALERT;
    const ups::UpsState state{ ups::UpsStatus::WARNING, deviations };
    EXPECT_EQ(output::formatState(state, m_timestamp),
              "2026-07-15 14:25:43 [UPS] status=WARNING deviations=0x0000006A");
}

#endif
