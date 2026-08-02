/**
 * @file ups_monitor_integration_test.cpp
 * @brief Интеграционный тест монитора ИБП с UPS-эмулятором.
 */
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

#include "snmp_client.h"
#include "snmp_ready_waiter.h"
#include "ups_emulator_process.h"
#include "ups_model_types.h"
#include "ups_monitor.h"

#ifndef UPS_EMULATOR_EXECUTABLE
#error "UPS_EMULATOR_EXECUTABLE is not defined"
#endif

namespace {

const char* const EMULATOR_MODEL = "APC_RT_2000_XL";
const char* const EMULATOR_HOST = "127.0.0.1";
const uint16_t EMULATOR_PORT = 1161;
const char* const EXPECTED_MODEL_NAME = "Smart-UPS RT 2000 XL";
const snmp::Oid MODEL_NAME_OID = "1.3.6.1.4.1.318.1.1.1.1.1.1.0";
const unsigned int READY_ATTEMPTS = 3;
const std::chrono::milliseconds READY_RETRY_INTERVAL(100);
const std::chrono::seconds STATE_TIMEOUT(3);
const std::chrono::milliseconds STATE_RETRY_INTERVAL(20);
const std::chrono::milliseconds STOP_TIMEOUT(500);

class UpsMonitorIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_emulator.reset(
            new UpsEmulatorProcess(UPS_EMULATOR_EXECUTABLE, EMULATOR_MODEL, EMULATOR_PORT));

        snmp::SnmpClient client(EMULATOR_HOST, EMULATOR_PORT);
        snmp::ErrorMessage error;
        ASSERT_TRUE(waitUntilSnmpReady(
            client, MODEL_NAME_OID, READY_ATTEMPTS, READY_RETRY_INTERVAL, &error))
            << error;
    }

    void TearDown() override { m_monitor.stop(); }

    bool waitForState(ups::UpsState& state) {
        const auto deadline = std::chrono::steady_clock::now() + STATE_TIMEOUT;
        while (std::chrono::steady_clock::now() < deadline) {
            if (m_monitor.tryConsumeState(state)) {
                return true;
            }
            std::this_thread::sleep_for(STATE_RETRY_INTERVAL);
        }
        return false;
    }

protected:
    std::unique_ptr<UpsEmulatorProcess> m_emulator;
    UpsMonitor m_monitor;
};

#if (1)  // Part 1 — полный цикл мониторинга UPS

// Test 1.1. Монитор определяет модель, получает ожидаемое состояние и штатно останавливается
TEST_F(UpsMonitorIntegrationTest, FullCycle_EmulatorAvailable_ReturnsStateAndStops) {
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init(EMULATOR_HOST, EMULATOR_PORT, error)) << error;
    EXPECT_EQ(m_monitor.modelName(), EXPECTED_MODEL_NAME);

    ups::UpsState state;
    ASSERT_TRUE(waitForState(state)) << "First UPS state was not received";
    EXPECT_EQ(state.status, ups::UpsStatus::WARNING);
    EXPECT_EQ(state.deviations,
              ups::UpsDeviationFlags::BATTERY_ALERT | ups::UpsDeviationFlags::BYPASS_ALERT);

    const auto stopStarted = std::chrono::steady_clock::now();
    m_monitor.stop();
    const auto stopElapsed = std::chrono::steady_clock::now() - stopStarted;
    EXPECT_LT(stopElapsed, STOP_TIMEOUT);
}

#endif

}  // namespace
