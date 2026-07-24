/**
 * @file ups_model_types.cpp
 * @ingroup ups
 * @brief Реализация вспомогательных функций доменной модели UPS.
 */
#include "ups_model_types.h"

#include <unordered_map>

namespace ups {

UpsStateDesc operator|(UpsStateDesc lhs, UpsStateDesc rhs) {
    return static_cast<UpsStateDesc>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

UpsStateDesc& operator|=(UpsStateDesc& lhs, UpsStateDesc rhs) {
    lhs = lhs | rhs;
    return lhs;
}

UpsStateDesc operator&(UpsStateDesc lhs, UpsStateDesc rhs) {
    return static_cast<UpsStateDesc>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

bool hasFlag(UpsStateDesc value, UpsStateDesc flag) {
    return (value & flag) != UpsStateDesc::NONE;
}

bool isFailureCause(UpsStateDesc desc) {
    switch (desc) {
        case UpsStateDesc::OUTPUT_FAILURE:
            return true;

        case UpsStateDesc::BATTERY_ALERT:
        case UpsStateDesc::CHARGE_ALERT:
        case UpsStateDesc::TEMP_ALERT:
        case UpsStateDesc::FREQ_ALERT:
        case UpsStateDesc::INPUT_ALERT:
        case UpsStateDesc::BYPASS_ALERT:
        case UpsStateDesc::NONE:
        default:
            return false;
    }
}

UpsStateDesc paramToDescMap(const ParamName& paramName) {
    // LCOV_EXCL_START // static map initialization is not tracked by gcov
    static const std::unordered_map<ParamName, UpsStateDesc> map = {
        { "batteryStatus", UpsStateDesc::BATTERY_ALERT },
        { "chargeRemaining", UpsStateDesc::CHARGE_ALERT },
        { "batteryTemp", UpsStateDesc::TEMP_ALERT },
        { "inputFreq", UpsStateDesc::FREQ_ALERT },
        { "inputVoltage", UpsStateDesc::INPUT_ALERT },
        { "outputVoltage", UpsStateDesc::OUTPUT_FAILURE },
        { "outputStatus", UpsStateDesc::BYPASS_ALERT },
    };
    // LCOV_EXCL_STOP

    auto it = map.find(paramName);
    if (it != map.end()) {
        return it->second;
    }

    return UpsStateDesc::NONE;
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
