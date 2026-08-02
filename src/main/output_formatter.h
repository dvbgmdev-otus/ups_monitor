/**
 * @file output_formatter.h
 * @brief Форматирование пользовательского вывода приложения.
 */
#ifndef OUTPUT_FORMATTER_H
#define OUTPUT_FORMATTER_H

#include <chrono>
#include <string>

namespace ups {
struct UpsState;
}

namespace output {

using Timestamp = std::chrono::system_clock::time_point;

/**
 * @brief Форматирует сообщение об обнаруженной модели ИБП.
 * @param modelName Название обнаруженной модели.
 * @param observedAt Момент, когда модель стала известна приложению.
 * @return Готовая строка без завершающего перевода строки.
 */
std::string formatDetectedModel(const std::string& modelName, const Timestamp& observedAt);

/**
 * @brief Форматирует полученное состояние ИБП.
 * @param state Полученное состояние ИБП.
 * @param observedAt Момент успешного получения состояния от монитора.
 * @return Готовая строка без завершающего перевода строки.
 */
std::string formatState(const ups::UpsState& state, const Timestamp& observedAt);

}  // namespace output

#endif  // OUTPUT_FORMATTER_H
