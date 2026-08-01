/**
 * @file ups_monitor_test.cpp
 * @brief Unit-тесты монитора состояния UPS.
 */

#include "ups_monitor.h"

#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>
#include <thread>

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

        void throwOnClientCreation() { m_throwOnClientCreation = true; }

        std::unique_ptr<snmp::ISnmpClient> createDefaultSnmpClient(const std::string& ip,
                                                                   uint16_t port) {
            return UpsMonitor::createSnmpClient(ip, port);
        }

        const std::string& clientIp() const { return m_clientIp; }
        uint16_t clientPort() const { return m_clientPort; }

    protected:
        std::unique_ptr<snmp::ISnmpClient> createSnmpClient(const std::string& ip,
                                                            uint16_t port) override {
            m_clientIp = ip;
            m_clientPort = port;
            if (m_throwOnClientCreation) {
                throw std::runtime_error("test error");
            }
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
        bool m_throwOnClientCreation{ false };
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

    bool waitForState(ups::UpsState& state, std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (m_monitor.tryConsumeState(state)) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        return m_monitor.tryConsumeState(state);
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

// Test 5.5: Исключение фабрики возвращается с контекстом создания SNMP-клиента
TEST_F(UpsMonitorTest, Init_ClientCreationThrows_ReturnsFalseWithContext) {
    m_monitor.throwOnClientCreation();
    ups::ErrorMessage error;
    EXPECT_FALSE(m_monitor.init("127.0.0.1", 161, error));
    EXPECT_EQ(error, "SNMP client creation failed: test error");
}

#endif

#if (1)  // Part 6 — Имя модели

// Test 6.1: До определения модели её имя недоступно
TEST_F(UpsMonitorTest, ModelName_ModelNotDetected_ReturnsEmptyString) {
    EXPECT_TRUE(m_monitor.modelName().empty());
}

// Test 6.2: После успешной инициализации возвращается имя обнаруженной модели
TEST_F(UpsMonitorTest, ModelName_Initialized_ReturnsDetectedModelName) {
    prepareValidModelResponse();
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    EXPECT_EQ(m_monitor.modelName(), "MP3000RT");
}

// Test 6.3: После остановки имя обнаруженной модели остаётся доступным
TEST_F(UpsMonitorTest, ModelName_AfterStop_ReturnsDetectedModelName) {
    prepareValidModelResponse();
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    m_monitor.stop();
    EXPECT_EQ(m_monitor.modelName(), "MP3000RT");
}

#endif

#if (1)  // Part 7 — Фоновый опрос

// Test 7.1: Первое состояние формируется без ожидания периода опроса
TEST_F(UpsMonitorTest, Polling_FirstState_IsAvailableImmediately) {
    prepareValidModelResponse();
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    ups::UpsState state;
    EXPECT_TRUE(waitForState(state, std::chrono::milliseconds(500)));
}

// Test 7.2: Остановка прерывает ожидание следующего опроса
TEST_F(UpsMonitorTest, Stop_WhileWaiting_CompletesBeforePollingPeriod) {
    prepareValidModelResponse();
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    ups::UpsState state;
    ASSERT_TRUE(waitForState(state, std::chrono::milliseconds(500)));
    const auto started = std::chrono::steady_clock::now();
    m_monitor.stop();
    const auto elapsed = std::chrono::steady_clock::now() - started;
    EXPECT_LT(elapsed, std::chrono::milliseconds(500));
}

// Test 7.3: После периода опроса формируется новое состояние
TEST_F(UpsMonitorTest, Polling_AfterPollingPeriod_ProducesNewState) {
    prepareValidModelResponse();
    ups::ErrorMessage error;
    ASSERT_TRUE(m_monitor.init("127.0.0.1", 161, error));
    ups::UpsState firstState;
    ASSERT_TRUE(waitForState(firstState, std::chrono::milliseconds(500)));
    ups::UpsState secondState;
    EXPECT_FALSE(m_monitor.tryConsumeState(secondState));
    EXPECT_TRUE(waitForState(secondState, std::chrono::milliseconds(1500)));
}

#endif

#if (1)  // Part 8 — Фабрика SNMP-клиента

// Test 8.1: Стандартная фабрика создаёт SNMP-клиент
TEST_F(UpsMonitorTest, CreateSnmpClient_ValidEndpoint_ReturnsClient) {
    std::unique_ptr<snmp::ISnmpClient> client =
        m_monitor.createDefaultSnmpClient("127.0.0.1", 161);
    EXPECT_NE(client, nullptr);
}

#endif
