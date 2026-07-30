/**
 * @file ups_state_poller.cpp
 * @ingroup ups
 * @brief Реализация опроса состояния ИБП по SNMP-данным.
 */
#include "ups_state_poller.h"

#include "snmp_client_iface.h"
#include "ups_model_spec.h"
#include "ups_model_types.h"
#include "ups_param_checker.h"

namespace ups {

UpsState UpsStatePoller::poll(const UpsModelSpec& spec, snmp::ISnmpClient& client) {
    UpsDeviationFlags deviations = UpsDeviationFlags::NONE;  // битовая маска отклонений
    bool hasSuccessfulResponse = false;  // Есть хотя бы один успешный SNMP-ответ.
    bool hasFailureCondition = false;    // Обнаружена проблема параметра, приводящего к Failure.
    bool hasWarningCondition = false;    // Обнаружена проблема параметра, не приводящего к Failure.

    // =========================================================
    // Шаг 1. Перебор параметров модели
    // =========================================================
    for (const auto& parameter : spec.parameters()) {
        const UpsParamSpec& parameterSpec = parameter.second;
        const UpsDeviationFlags deviationFlag = toDeviationFlag(parameterSpec.name);
        const bool isFailureParameter = isFailureCause(deviationFlag);
        bool hasParameterCondition = false;

        // Делаем SNMP GET
        snmp::codec::SnmpValue snmpValue;
        if (!client.get(parameterSpec.oid, snmpValue, nullptr)) {
            // SNMP GET завершился ошибкой
            hasParameterCondition = true;
        } else {
            // SNMP GET выполнен успешно, есть ответ от ИБП
            hasSuccessfulResponse = true;
            // Проверяем значение и накапливаем обнаруженные отклонения.
            UpsDeviationFlags parameterDeviations = UpsDeviationFlags::NONE;
            const bool isValueSupported =
                UpsParamChecker::check(parameterSpec, snmpValue, parameterDeviations);
            deviations |= parameterDeviations;
            const bool hasParameterDeviation = hasFlag(parameterDeviations, deviationFlag);
            hasParameterCondition = !isValueSupported || hasParameterDeviation;
        }

        if (!hasParameterCondition) {
            continue;
        }

        if (isFailureParameter) {
            hasFailureCondition = true;
        } else {
            hasWarningCondition = true;
        }
    }

    // =========================================================
    // Шаг 2. Формирование итогового статуса
    // =========================================================
    UpsStatus status = UpsStatus::OK;
    if (!hasSuccessfulResponse) {
        status = UpsStatus::NO_INFO;
    } else if (hasFailureCondition) {
        status = UpsStatus::FAILURE;
    } else if (hasWarningCondition) {
        status = UpsStatus::WARNING;
    } else {
        status = UpsStatus::OK;
    }

    // =========================================================
    // Шаг 3. Возврат состояния
    // =========================================================
    UpsState state;
    state.status = status;
    state.deviations = deviations;
    return state;
}

}  // namespace ups
