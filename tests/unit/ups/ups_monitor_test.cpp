/**
 * @file ups_monitor_test.cpp
 * @brief Unit-тесты монитора состояния UPS.
 */

#include "ups_monitor.h"

#include <gtest/gtest.h>

#include "ups_model_types.h"

class UpsMonitorTest : public ::testing::Test {
protected:
    class TestUpsMonitor : public UpsMonitor {
    public:
        void storeState(const ups::UpsState& state) { m_stateBuffer.storeState(state); }
    };

    TestUpsMonitor m_monitor;
};

#if (1)  // Part 1 — Пустой монитор

// Test 1.1: При отсутствии состояния метод возвращает false
TEST_F(UpsMonitorTest, TryConsumeState_NoState_ReturnsFalse) {
    ups::UpsState state;
    EXPECT_FALSE(m_monitor.tryConsumeState(state));
}

#endif

#if (1)  // Part 2 — Получение состояния

// Test 2.1: Сохранённое состояние возвращается без изменений
TEST_F(UpsMonitorTest, TryConsumeState_StateAvailable_ReturnsState) {
    const ups::UpsDeviationFlags deviations =
        ups::UpsDeviationFlags::BATTERY_ALERT | ups::UpsDeviationFlags::CHARGE_ALERT;
    ups::UpsState storedState;
    storedState.status = ups::UpsStatus::WARNING;
    storedState.deviations = deviations;
    m_monitor.storeState(storedState);
    ups::UpsState state;
    ASSERT_TRUE(m_monitor.tryConsumeState(state));
    EXPECT_EQ(state.status, ups::UpsStatus::WARNING);
    EXPECT_EQ(state.deviations, deviations);
}

#endif

#if (1)  // Part 3 — Однократное потребление

// Test 3.1: Повторное получение уже потреблённого состояния возвращает false
TEST_F(UpsMonitorTest, TryConsumeState_StateAlreadyConsumed_ReturnsFalse) {
    ups::UpsState storedState;
    storedState.status = ups::UpsStatus::OK;
    storedState.deviations = ups::UpsDeviationFlags::NONE;
    m_monitor.storeState(storedState);
    ups::UpsState state;
    ASSERT_TRUE(m_monitor.tryConsumeState(state));
    EXPECT_FALSE(m_monitor.tryConsumeState(state));
}

#endif
