/**
 * @file output_formatter.cpp
 * @brief Реализация форматирования пользовательского вывода приложения.
 */
#include "output_formatter.h"

#include <array>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

#include "ups_model_types.h"

namespace {

std::string formatLocalTime(const output::Timestamp& timestamp) {
    const std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm localTime{};
    localtime_r(&time, &localTime);

    char buffer[20]{};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
}

std::string formatDeviationFlags(ups::UpsDeviationFlags deviations) {
    if (deviations == ups::UpsDeviationFlags::NONE) {
        return "NONE";
    }

    using DeviationFlagName = std::pair<ups::UpsDeviationFlags, const char*>;
    static const std::array<DeviationFlagName, 7> flags = { {
        { ups::UpsDeviationFlags::BATTERY_ALERT, "BATTERY_ALERT" },
        { ups::UpsDeviationFlags::CHARGE_ALERT, "CHARGE_ALERT" },
        { ups::UpsDeviationFlags::TEMP_ALERT, "TEMP_ALERT" },
        { ups::UpsDeviationFlags::FREQ_ALERT, "FREQ_ALERT" },
        { ups::UpsDeviationFlags::INPUT_ALERT, "INPUT_ALERT" },
        { ups::UpsDeviationFlags::OUTPUT_FAILURE, "OUTPUT_FAILURE" },
        { ups::UpsDeviationFlags::BYPASS_ALERT, "BYPASS_ALERT" },
    } };

    std::ostringstream stream;
    bool first = true;
    for (const auto& flag : flags) {
        if (!ups::hasFlag(deviations, flag.first)) {
            continue;
        }
        if (!first) {
            stream << " | ";
        }
        stream << flag.second;
        first = false;
    }
    return stream.str();
}

}  // namespace

namespace output {

std::string formatDetectedModel(const std::string& modelName, const Timestamp& observedAt) {
    std::ostringstream stream;
    stream << formatLocalTime(observedAt) << " [UPS] detected model=\"" << modelName << '"';
    return stream.str();
}

std::string formatState(const ups::UpsState& state, const Timestamp& observedAt) {
    std::ostringstream stream;
    stream << formatLocalTime(observedAt) << " [UPS] status=" << ups::toString(state.status)
           << " deviations=0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0')
           << static_cast<uint32_t>(state.deviations) << " ("
           << formatDeviationFlags(state.deviations) << ')';
    return stream.str();
}

}  // namespace output
