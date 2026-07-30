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
    UpsDeviationFlags descr = UpsDeviationFlags::NONE;  // битовая маска отклонений

    bool anyValid = false;         // есть хотя бы один валидный ответ
    bool criticalFailure = false;  // критичный параметр вне допуска
    bool criticalNoInfo = false;   // нет данных по критичному параметру
    bool warningPresent = false;   // некритичный параметр вне допуска или NoInfo

    // =========================================================
    // Шаг 1. Перебор параметров модели
    // =========================================================
    const auto& params = spec.parameters();

    for (const auto& kv : params) {
        const UpsParamSpec& paramSpec = kv.second;

        const UpsDeviationFlags desc = toDeviationFlag(paramSpec.name);
        const bool isCritical = isFailureCause(desc);

        // -----------------------------------------------------
        // Шаг 2. SNMP GET
        // -----------------------------------------------------
        snmp::codec::SnmpValue value;
        if (!client.get(paramSpec.oid, value, nullptr)) {
            // Нет данных (NoInfo)
            if (isCritical) {
                criticalNoInfo = true;
            } else {
                warningPresent = true;
            }
            continue;
        }

        anyValid = true;

        // -----------------------------------------------------
        // Шаг 3. Проверка наличия параметра
        // -----------------------------------------------------
        UpsDeviationFlags paramDescr = UpsDeviationFlags::NONE;
        const bool hasData = UpsParamChecker::check(paramSpec, value, paramDescr);

        // аккумулируем диагностические биты
        descr |= paramDescr;

        if (!hasData) {
            // Данные не получены или невалидны (NoInfo)
            if (isCritical) {
                criticalNoInfo = true;
            } else {
                warningPresent = true;
            }
            continue;
        }

        // -----------------------------------------------------
        // Шаг 4. Анализ выхода за допуск
        // -----------------------------------------------------
        // UpsParamChecker::check() устанавливает бит в paramDescr,
        // если параметр вышел за допустимые пределы
        const UpsDeviationFlags descBit = desc;
        const bool isOutOfRange = (paramDescr & descBit) != UpsDeviationFlags::NONE;

        if (isOutOfRange) {
            if (isCritical) {
                criticalFailure = true;
            } else {
                warningPresent = true;
            }
        }
    }

    // =========================================================
    // Шаг 5. Формирование итогового статуса
    // =========================================================
    UpsStatus status = UpsStatus::OK;

    if (!anyValid) {
        status = UpsStatus::NO_INFO;
    } else if (criticalFailure || criticalNoInfo) {
        status = UpsStatus::FAILURE;
    } else if (warningPresent) {
        status = UpsStatus::WARNING;
    } else {
        status = UpsStatus::OK;
    }

    // =========================================================
    // Шаг 6. Возврат состояния
    // =========================================================
    UpsState outputState;
    outputState.status = status;
    outputState.deviations = descr;
    return outputState;
}

}  // namespace ups
