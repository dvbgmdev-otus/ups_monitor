/**
 * @file ups_state_poller.cpp
 * @ingroup ups
 * @brief Реализация опроса состояния UPS по SNMP-данным.
 */
#include "ups_state_poller.h"

#include "snmp_client_iface.h"
#include "ups_model_spec.h"
#include "ups_model_types.h"
#include "ups_param_checker.h"

namespace ups {

UpsState UpsStatePoller::poll(const UpsModelSpec& spec, snmp::ISnmpClient& client) {
    // =========================================================
    // Шаг 0. Инициализация
    // =========================================================
    UpsDeviationFlags deviations = UpsDeviationFlags::NONE;  // битовая маска отклонений

    bool hasSuccessfulResponse = false;  // есть хотя бы один успешный SNMP-ответ
    bool hasFailureCondition = false;  // параметр, приводящий к Failure, недоступен или вне допуска
    bool hasWarningCondition =
        false;  // параметр, не приводящий к Failure, недоступен или вне допуска

    // =========================================================
    // Шаг 1. Перебор параметров модели
    // =========================================================
    for (const auto& parameter : spec.parameters()) {
        const UpsParamSpec& parameterSpec = parameter.second;
        const UpsDeviationFlags deviationFlag = toDeviationFlag(parameterSpec.name);
        const bool isFailureParameter = isFailureCause(deviationFlag);

        // -----------------------------------------------------
        // Шаг 2. SNMP GET
        // -----------------------------------------------------
        snmp::codec::SnmpValue snmpValue;
        if (!client.get(parameterSpec.oid, snmpValue, nullptr)) {
            // SNMP GET завершился ошибкой
            if (isFailureParameter) {
                hasFailureCondition = true;
            } else {
                hasWarningCondition = true;
            }
            continue;
        }

        hasSuccessfulResponse = true;

        // -----------------------------------------------------
        // Шаг 3. Проверка значения параметра
        // -----------------------------------------------------
        UpsDeviationFlags parameterDeviations = UpsDeviationFlags::NONE;
        const bool isValueSupported =
            UpsParamChecker::check(parameterSpec, snmpValue, parameterDeviations);

        // аккумулируем флаги отклонений
        deviations |= parameterDeviations;

        if (!isValueSupported) {
            // Тип значения не поддерживается
            if (isFailureParameter) {
                hasFailureCondition = true;
            } else {
                hasWarningCondition = true;
            }
            continue;
        }

        // -----------------------------------------------------
        // Шаг 4. Анализ выхода за допуск
        // -----------------------------------------------------
        // UpsParamChecker::check() устанавливает бит в parameterDeviations,
        // если параметр вышел за допустимые пределы
        const bool hasDeviation = hasFlag(parameterDeviations, deviationFlag);

        if (hasDeviation) {
            if (isFailureParameter) {
                hasFailureCondition = true;
            } else {
                hasWarningCondition = true;
            }
        }
    }

    // =========================================================
    // Шаг 5. Формирование итогового статуса
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
    // Шаг 6. Возврат состояния
    // =========================================================
    UpsState state;
    state.status = status;
    state.deviations = deviations;
    return state;
}

}  // namespace ups
