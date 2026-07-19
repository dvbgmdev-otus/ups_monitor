/**
 * @file snmp_client_integration_test.cpp
 * @brief Интеграционные тесты SNMP-клиента с UPS-эмулятором.
 */
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>

#include "snmp_client.h"
#include "snmp_ready_waiter.h"
#include "ups_emulator_process.h"

#ifndef UPS_EMULATOR_EXECUTABLE
#error "UPS_EMULATOR_EXECUTABLE is not defined"
#endif

namespace {

const char* const EMULATOR_MODEL = "APC_RT_2000_XL";
const char* const EMULATOR_HOST = "127.0.0.1";
const uint16_t EMULATOR_PORT = 1161;
const snmp::Oid MODEL_NAME_OID = "1.3.6.1.4.1.318.1.1.1.1.1.1.0";
const char* const EXPECTED_MODEL_NAME = "Smart-UPS RT 2000 XL";
const unsigned int READY_ATTEMPTS = 3;
const std::chrono::milliseconds READY_RETRY_INTERVAL(100);

class SnmpClientIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_emulator.reset(
            new UpsEmulatorProcess(UPS_EMULATOR_EXECUTABLE, EMULATOR_MODEL, EMULATOR_PORT));
        m_client.reset(new snmp::SnmpClient(EMULATOR_HOST, EMULATOR_PORT));

        snmp::ErrorMessage error;
        ASSERT_TRUE(waitUntilSnmpReady(
            *m_client, MODEL_NAME_OID, READY_ATTEMPTS, READY_RETRY_INTERVAL, &error))
            << error;
    }

protected:
    std::unique_ptr<UpsEmulatorProcess> m_emulator;
    std::unique_ptr<snmp::SnmpClient> m_client;
};

#if (1)  // Часть 1 — успешный SNMP GET через эмулятор

// 1.1. Клиент получает название выбранной модели ИБП
TEST_F(SnmpClientIntegrationTest, GetModelNameReturnsExpectedString) {
    snmp::codec::SnmpValue value;
    snmp::ErrorMessage error;

    ASSERT_TRUE(m_client->get(MODEL_NAME_OID, value, &error)) << error;
    EXPECT_EQ(value.oid, MODEL_NAME_OID);
    EXPECT_EQ(value.type, snmp::codec::SnmpValue::Type::String);
    EXPECT_EQ(value.strValue, EXPECTED_MODEL_NAME);
}

#endif

}  // namespace
