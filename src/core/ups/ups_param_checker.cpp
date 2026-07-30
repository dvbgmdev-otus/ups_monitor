/**
 * @file ups_param_checker.cpp
 * @ingroup ups
 * @brief Реализация проверки параметров UPS по спецификации модели.
 */
#include "ups_param_checker.h"

#include <algorithm>

namespace ups {

bool UpsParamChecker::check(const UpsParamSpec& spec,
                            const snmp::codec::SnmpValue& value,
                            UpsDeviationFlags& deviations) {
    // Спецификация с неизвестным именем параметра здесь логически недопустима
    // считаем, что контракт соблюдён и toDeviationFlag() не вернёт NONE
    // при желании можно добавить assert

    // -------------------------------------------------
    // 1. Проверка наличия и типа данных
    // -------------------------------------------------
    if (value.type != snmp::codec::SnmpValue::Type::Integer) {
        // Нет данных или тип не поддерживается
        return false;
    }

    const bool hasNormal = spec.normal.isRange || !spec.normal.values.empty();
    // Критерии normal и bypass беззнаковые, поэтому отрицательное значение
    // не включает байпас и при наличии normal считается отклонением.
    if (value.intValue < 0) {
        if (hasNormal) {
            deviations |= toDeviationFlag(spec.name);
        }
        return true;
    }

    const uint32_t paramValue = static_cast<uint32_t>(value.intValue);

    // -------------------------------------------------
    // 2. Проверка bypass
    // -------------------------------------------------
    if (!spec.bypass.empty()) {
        const auto it = std::find(spec.bypass.begin(), spec.bypass.end(), paramValue);
        if (it != spec.bypass.end()) {
            deviations |= UpsDeviationFlags::BYPASS_ALERT;
        }
    }

    // -------------------------------------------------
    // 3. Проверка normal
    // -------------------------------------------------
    if (hasNormal) {
        bool inNormal = false;
        if (spec.normal.isRange) {
            inNormal = paramValue >= spec.normal.min && paramValue <= spec.normal.max;
        } else {
            inNormal = std::find(spec.normal.values.begin(),
                                 spec.normal.values.end(),
                                 paramValue) != spec.normal.values.end();
        }

        if (!inNormal) {
            deviations |= toDeviationFlag(spec.name);
        }
    }

    return true;
}

}  // namespace ups
