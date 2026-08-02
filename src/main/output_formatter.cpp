/**
 * @file output_formatter.cpp
 * @brief Реализация форматирования пользовательского вывода приложения.
 */
#include "output_formatter.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace {

std::string formatLocalTime(const output::Timestamp& timestamp) {
    const std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm localTime{};
    localtime_r(&time, &localTime);

    char buffer[20]{};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return buffer;
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
           << static_cast<uint32_t>(state.deviations);
    return stream.str();
}

}  // namespace output
