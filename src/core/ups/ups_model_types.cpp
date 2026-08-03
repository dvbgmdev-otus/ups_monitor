/**
 * @file ups_model_types.cpp
 * @ingroup ups
 * @brief Реализация вспомогательных функций доменной модели UPS.
 */
#include "ups_model_types.h"

#include <unordered_map>

namespace ups {

UpsDeviationFlags operator|(UpsDeviationFlags lhs, UpsDeviationFlags rhs) {
    return static_cast<UpsDeviationFlags>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

UpsDeviationFlags& operator|=(UpsDeviationFlags& lhs, UpsDeviationFlags rhs) {
    lhs = lhs | rhs;
    return lhs;
}

UpsDeviationFlags operator&(UpsDeviationFlags lhs, UpsDeviationFlags rhs) {
    return static_cast<UpsDeviationFlags>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

bool hasFlag(UpsDeviationFlags value, UpsDeviationFlags flag) {
    return (value & flag) != UpsDeviationFlags::NONE;
}

bool isFailureCause(UpsDeviationFlags flag) {
    switch (flag) {
        case UpsDeviationFlags::OUTPUT_FAILURE:
            return true;

        case UpsDeviationFlags::BATTERY_ALERT:
        case UpsDeviationFlags::CHARGE_ALERT:
        case UpsDeviationFlags::TEMP_ALERT:
        case UpsDeviationFlags::FREQ_ALERT:
        case UpsDeviationFlags::INPUT_ALERT:
        case UpsDeviationFlags::BYPASS_ALERT:
        case UpsDeviationFlags::NONE:
        default:
            return false;
    }
}

UpsDeviationFlags toDeviationFlag(const ParamName& paramName) {
    // LCOV_EXCL_START // static map initialization is not tracked by gcov
    static const std::unordered_map<ParamName, UpsDeviationFlags> map = {
        { "batteryStatus", UpsDeviationFlags::BATTERY_ALERT },
        { "chargeRemaining", UpsDeviationFlags::CHARGE_ALERT },
        { "batteryTemp", UpsDeviationFlags::TEMP_ALERT },
        { "inputFreq", UpsDeviationFlags::FREQ_ALERT },
        { "inputVoltage", UpsDeviationFlags::INPUT_ALERT },
        { "outputVoltage", UpsDeviationFlags::OUTPUT_FAILURE },
        { "outputStatus", UpsDeviationFlags::BYPASS_ALERT },
    };
    // LCOV_EXCL_STOP

    auto it = map.find(paramName);
    if (it != map.end()) {
        return it->second;
    }

    return UpsDeviationFlags::NONE;
}

const char* toString(UpsStatus status) {
    switch (status) {
        case UpsStatus::OK:
            return "OK";

        case UpsStatus::WARNING:
            return "WARNING";

        case UpsStatus::FAILURE:
            return "FAILURE";

        case UpsStatus::NO_INFO:
            return "NO_INFO";

        default:
            return "UNKNOWN";
    }
}

}  // namespace ups
