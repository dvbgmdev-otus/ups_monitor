/**
 * @file ups_state_poller_test.cpp
 * @brief Unit-тесты одного цикла опроса состояния ИБП.
 *
 * Проверяется:
 *  - формирование состояний OK, WARNING, FAILURE и NO_INFO;
 *  - обработка ошибок SNMP GET и неподдерживаемых типов;
 *  - накопление флагов отклонений;
 *  - продолжение опроса после ошибки отдельного параметра.
 */

#include "ups_state_poller.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "fake_snmp_client.h"
#include "temp_ini_file.h"
#include "ups_model_spec.h"
#include "ups_model_types.h"

class UpsStatePollerTest : public ::testing::Test {
protected:
    void SetUp() override {
        const std::string iniPath = m_tempIniFiles.write(R"(
[TEST]
modelName = TEST_UPS
modelName.oid = 1.2.3.0

batteryStatus.oid = 1.2.3.1
batteryStatus.normal = 2

chargeRemaining.oid = 1.2.3.2
chargeRemaining.normal = 30..100

batteryTemp.oid = 1.2.3.3
batteryTemp.normal = 0..50

inputFreq.oid = 1.2.3.4
inputFreq.normal = 40..60

inputVoltage.oid = 1.2.3.5
inputVoltage.normal = 200..259

outputStatus.oid = 1.2.3.6
outputStatus.bypass = 6,9,10

outputVoltage.oid = 1.2.3.7
outputVoltage.normal = 200..259
)");
        ASSERT_TRUE(m_spec.load(iniPath, "TEST")) << m_spec.lastError();
        setNormalResponses();
    }

    static snmp::codec::SnmpValue makeIntValue(int value) {
        snmp::codec::SnmpValue result;
        result.type = snmp::codec::SnmpValue::Type::Integer;
        result.intValue = value;
        return result;
    }

    static snmp::codec::SnmpValue makeStringValue(const std::string& value) {
        snmp::codec::SnmpValue result;
        result.type = snmp::codec::SnmpValue::Type::String;
        result.strValue = value;
        return result;
    }

    void setNormalResponses() {
        for (const auto& parameter : m_spec.parameters()) {
            const ups::UpsParamSpec& parameterSpec = parameter.second;
            uint32_t value = 1;
            if (parameterSpec.normal.isRange) {
                value = parameterSpec.normal.min;
            } else if (!parameterSpec.normal.values.empty()) {
                value = parameterSpec.normal.values.front();
            }
            m_snmp.set(parameterSpec.oid, { true, makeIntValue(value), {} });
        }
    }

    void setResponse(const ups::ParamName& parameterName,
                     const test::FakeSnmpClient::Response& response) {
        m_snmp.set(m_spec.parameters().at(parameterName).oid, response);
    }

    void expectState(ups::UpsStatus expectedStatus, ups::UpsDeviationFlags expectedDeviations) {
        const ups::UpsState state = ups::UpsStatePoller::poll(m_spec, m_snmp);
        EXPECT_EQ(state.status, expectedStatus);
        EXPECT_EQ(state.deviations, expectedDeviations);
    }

    test::FakeSnmpClient m_snmp;
    ups::UpsModelSpec m_spec;
    test::TempIniFileStorage m_tempIniFiles;
};

#if (1)  // Part 1 — Общие состояния

// Test 1.1: Полностью недоступный ИБП возвращает NO_INFO без флагов отклонений
TEST_F(UpsStatePollerTest, Poll_AllRequestsFail_ReturnsNoInfoWithoutDeviations) {
    m_snmp = test::FakeSnmpClient{};
    expectState(ups::UpsStatus::NO_INFO, ups::UpsDeviationFlags::NONE);
}

// Test 1.2: Все параметры в норме возвращают OK без флагов отклонений
TEST_F(UpsStatePollerTest, Poll_AllParametersNormal_ReturnsOkWithoutDeviations) {
    expectState(ups::UpsStatus::OK, ups::UpsDeviationFlags::NONE);
}

#endif

#if (1)  // Part 2 — Предупреждающие условия

// Test 2.1: Некритичное отклонение возвращает WARNING с соответствующим флагом
TEST_F(UpsStatePollerTest, Poll_NonFailureParameterOutOfRange_ReturnsWarningWithDeviation) {
    setResponse("inputVoltage", { true, makeIntValue(150), {} });
    expectState(ups::UpsStatus::WARNING, ups::UpsDeviationFlags::INPUT_ALERT);
}

// Test 2.2: Недоступный некритичный параметр возвращает WARNING без дополнительного флага
TEST_F(UpsStatePollerTest, Poll_NonFailureParameterUnavailable_ReturnsWarningWithoutDeviation) {
    setResponse("inputVoltage", { false, {}, "SNMP request failed" });
    expectState(ups::UpsStatus::WARNING, ups::UpsDeviationFlags::NONE);
}

// Test 2.3: Неподдерживаемый тип некритичного параметра возвращает WARNING без флага
TEST_F(UpsStatePollerTest, Poll_NonFailureParameterTypeUnsupported_ReturnsWarningWithoutDeviation) {
    setResponse("inputVoltage", { true, makeStringValue("invalid"), {} });
    expectState(ups::UpsStatus::WARNING, ups::UpsDeviationFlags::NONE);
}

// Test 2.4: Включённый байпас возвращает WARNING с флагом BYPASS_ALERT
TEST_F(UpsStatePollerTest, Poll_BypassEnabled_ReturnsWarningWithBypassAlert) {
    setResponse("outputStatus", { true, makeIntValue(6), {} });
    expectState(ups::UpsStatus::WARNING, ups::UpsDeviationFlags::BYPASS_ALERT);
}

#endif

#if (1)  // Part 3 — Аварийные условия

// Test 3.1: Выходное напряжение вне нормы возвращает FAILURE с флагом OUTPUT_FAILURE
TEST_F(UpsStatePollerTest, Poll_OutputVoltageOutOfRange_ReturnsFailureWithDeviation) {
    setResponse("outputVoltage", { true, makeIntValue(150), {} });
    expectState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
}

// Test 3.2: Недоступное выходное напряжение возвращает FAILURE без дополнительного флага
TEST_F(UpsStatePollerTest, Poll_OutputVoltageUnavailable_ReturnsFailureWithoutDeviation) {
    setResponse("outputVoltage", { false, {}, "SNMP request failed" });
    expectState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::NONE);
}

// Test 3.3: Неподдерживаемый тип выходного напряжения возвращает FAILURE без флага
TEST_F(UpsStatePollerTest, Poll_OutputVoltageTypeUnsupported_ReturnsFailureWithoutDeviation) {
    setResponse("outputVoltage", { true, makeStringValue("invalid"), {} });
    expectState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::NONE);
}

// Test 3.4: FAILURE имеет приоритет, а критичный и некритичный флаги объединяются
TEST_F(UpsStatePollerTest, Poll_FailureAndWarningDeviations_ReturnsFailureWithCombinedMask) {
    setResponse("inputVoltage", { true, makeIntValue(150), {} });
    setResponse("outputVoltage", { true, makeIntValue(150), {} });
    const ups::UpsDeviationFlags expected =
        ups::UpsDeviationFlags::INPUT_ALERT | ups::UpsDeviationFlags::OUTPUT_FAILURE;
    expectState(ups::UpsStatus::FAILURE, expected);
}

#endif

#if (1)  // Part 4 — Полнота цикла опроса

// Test 4.1: Ошибка одного параметра не мешает обработать последующее критичное отклонение
TEST_F(UpsStatePollerTest, Poll_ParameterRequestFails_ContinuesPollingRemainingParameters) {
    setResponse("batteryStatus", { false, {}, "SNMP request failed" });
    setResponse("outputVoltage", { true, makeIntValue(150), {} });
    expectState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::OUTPUT_FAILURE);
}

// Test 4.2: Единственный успешный некритичный ответ даёт FAILURE при недоступном outputVoltage
TEST_F(UpsStatePollerTest, Poll_OnlyNonFailureResponseSucceeds_ReturnsFailureNotNoInfo) {
    m_snmp = test::FakeSnmpClient{};
    setResponse("inputVoltage", { true, makeIntValue(230), {} });
    expectState(ups::UpsStatus::FAILURE, ups::UpsDeviationFlags::NONE);
}

#endif
