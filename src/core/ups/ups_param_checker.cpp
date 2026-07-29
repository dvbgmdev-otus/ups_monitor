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
                            UpsDeviationFlags deviation,
                            uint32_t& descr) {
    // UpsDeviationFlags::NONE здесь логически недопустим
    // считаем, что контракт соблюдён
    // при желании можно добавить assert

    // -------------------------------------------------
    // 1. Проверка наличия и типа данных
    // -------------------------------------------------
    if (value.type != snmp::codec::SnmpValue::Type::Integer) {
        // Нет данных или тип невалиден
        return false;
    }

    const uint32_t v = static_cast<uint32_t>(value.intValue);

    // -------------------------------------------------
    // 2. Проверка bypass
    // -------------------------------------------------
    if (!spec.bypass.empty()) {
        auto it = std::find(spec.bypass.begin(), spec.bypass.end(), v);
        if (it != spec.bypass.end()) {
            descr |= static_cast<uint32_t>(UpsDeviationFlags::BYPASS_ALERT);
        }
        return true;
    }

    // -------------------------------------------------
    // 3. Проверка normal
    // -------------------------------------------------
    bool inNormal = false;

    if (spec.normal.isRange) {
        inNormal = (v >= spec.normal.min && v <= spec.normal.max);
    } else {
        inNormal = std::find(spec.normal.values.begin(), spec.normal.values.end(), v) !=
                   spec.normal.values.end();
    }

    if (!inNormal) {
        descr |= static_cast<uint32_t>(deviation);
    }

    return true;
}

}  // namespace ups
