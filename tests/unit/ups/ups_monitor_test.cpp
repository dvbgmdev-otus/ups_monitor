/**
 * @file ups_monitor_test.cpp
 * @brief Unit-тесты монитора состояния UPS.
 */

#include "ups_monitor.h"

#include <gtest/gtest.h>

#include "fake_snmp_client.h"
#include "ups_model_spec.h"
#include "ups_model_types.h"

class UpsMonitorTest : public ::testing::Test {
protected:
    class TestUpsMonitor : public UpsMonitor {
    public:
        void storeState(const ups::UpsState& state) { m_stateBuffer.storeState(state); }

        void prepareModelResponse(const snmp::Oid& oid, const snmp::codec::SnmpValue& value) {
            m_modelOid = oid;
            m_modelValue = value;
            m_hasModelResponse = true;
        }

        const std::string& clientIp() const { return m_clientIp; }
        uint16_t clientPort() const { return m_clientPort; }

    protected:
        std::unique_ptr<snmp::ISnmpClient> createSnmpClient(const std::string& ip,
                                                            uint16_t port) override {
            m_clientIp = ip;
            m_clientPort = port;
            std::unique_ptr<test::FakeSnmpClient> client(new test::FakeSnmpClient());
            if (m_hasModelResponse) {
                client->set(m_modelOid, { true, m_modelValue, "" });
            }
            return std::unique_ptr<snmp::ISnmpClient>(client.release());
        }

    private:
        std::string m_clientIp;
        uint16_t m_clientPort{ 0 };
        bool m_hasModelResponse{ false };
        snmp::Oid m_modelOid;
        snmp::codec::SnmpValue m_modelValue;
    };

    void prepareValidModelResponse() {
        ups::UpsModelSpec spec;
        ASSERT_TRUE(spec.load("../config/ups_model_spec.ini", "INELT_MP3000RT"));

        snmp::codec::SnmpValue value;
        value.type = snmp::codec::SnmpValue::Type::String;
        value.strValue = spec.modelName();
        m_monitor.prepareModelResponse(spec.modelNameOid(), value);
    }

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

#if (1)  // Part 4 — Остановка

// Test 4.1: Остановка неинициализированного монитора не выбрасывает исключений
TEST_F(UpsMonitorTest, Stop_NotInitialized_DoesNothing) { EXPECT_NO_THROW(m_monitor.stop()); }

// Test 4.2: Повторная остановка монитора не выбрасывает исключений
TEST_F(UpsMonitorTest, Stop_CalledTwice_DoesNothing) {
    ASSERT_NO_THROW(m_monitor.stop());
    EXPECT_NO_THROW(m_monitor.stop());
}

#endif

#if (1)  // Part 5 — Инициализация

// Test 5.1: Успешная инициализация передаёт IP и порт фабрике SNMP-клиента
TEST_F(UpsMonitorTest, Init_ValidModel_ReturnsTrueAndPassesEndpoint) {
    prepareValidModelResponse();
    ups::ErrorMessage error = "previous error";
    ASSERT_TRUE(m_monitor.init("192.0.2.1", 1161, error));
    EXPECT_TRUE(error.empty());
    EXPECT_EQ(m_monitor.clientIp(), "192.0.2.1");
    EXPECT_EQ(m_monitor.clientPort(), 1161);
}

// Test 5.2: Ошибка определения модели возвращается через выходной параметр
TEST_F(UpsMonitorTest, Init_ModelNotDetected_ReturnsFalseWithError) {
    ups::ErrorMessage error;
    EXPECT_FALSE(m_monitor.init("127.0.0.1", 161, error));
    EXPECT_FALSE(error.empty());
}

// Test 5.3: Повторная инициализация после остановки запрещена
TEST_F(UpsMonitorTest, Init_AfterSuccessfulInitAndStop_ReturnsFalse) {
    prepareValidModelResponse();
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    m_monitor.stop();
    EXPECT_FALSE(m_monitor.init("127.0.0.1", 161, error));
    EXPECT_FALSE(error.empty());
}

// Test 5.4: Повторная инициализация после ошибки разрешена
TEST_F(UpsMonitorTest, Init_AfterFailedInit_CanSucceed) {
    ups::ErrorMessage error;
    ASSERT_FALSE(m_monitor.init("127.0.0.1", 161, error));
    prepareValidModelResponse();
    EXPECT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    EXPECT_TRUE(error.empty());
}

#endif
