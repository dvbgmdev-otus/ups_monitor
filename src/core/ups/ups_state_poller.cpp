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

    bool hasSuccessfulResponse = false;  // есть хотя бы один валидный ответ
    bool criticalFailure = false;        // критичный параметр вне допуска
    bool criticalNoInfo = false;         // нет данных по критичному параметру
    bool hasWarning = false;             // некритичный параметр вне допуска или NoInfo

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
            // Нет данных (NoInfo)
            if (isFailureParameter) {
                criticalNoInfo = true;
            } else {
                hasWarning = true;
            }
            continue;
        }

        hasSuccessfulResponse = true;

        // -----------------------------------------------------
        // Шаг 3. Проверка наличия параметра
        // -----------------------------------------------------
        UpsDeviationFlags parameterDeviations = UpsDeviationFlags::NONE;
        const bool isValueSupported = UpsParamChecker::check(parameterSpec, snmpValue, parameterDeviations);

        // аккумулируем диагностические биты
        deviations |= parameterDeviations;

        if (!isValueSupported) {
            // Данные не получены или невалидны (NoInfo)
            if (isFailureParameter) {
                criticalNoInfo = true;
            } else {
                hasWarning = true;
            }
            continue;
        }

        // -----------------------------------------------------
        // Шаг 4. Анализ выхода за допуск
        // -----------------------------------------------------
        // UpsParamChecker::check() устанавливает бит в parameterDeviations,
        // если параметр вышел за допустимые пределы
        const bool hasDeviation = (parameterDeviations & deviationFlag) != UpsDeviationFlags::NONE;

        if (hasDeviation) {
            if (isFailureParameter) {
                criticalFailure = true;
            } else {
                hasWarning = true;
            }
        }
    }

    // =========================================================
    // Шаг 5. Формирование итогового статуса
    // =========================================================
    UpsStatus status = UpsStatus::OK;

    if (!hasSuccessfulResponse) {
        status = UpsStatus::NO_INFO;
    } else if (criticalFailure || criticalNoInfo) {
        status = UpsStatus::FAILURE;
    } else if (hasWarning) {
        status = UpsStatus::WARNING;
    } else {
        status = UpsStatus::OK;
    }

    // =========================================================
    // Шаг 6. Возврат состояния
    // =========================================================
    UpsState outputState;
    outputState.status = status;
    outputState.deviations = deviations;
    return outputState;
}

}  // namespace ups
