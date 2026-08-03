/**
 * @file string_utils.h
 * @ingroup utils
 * @brief Утилиты для работы со строками.
 *
 * Содержит вспомогательные функции для обработки строк.
 */
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>

namespace utils {

/**
 * @brief Удаляет пробельные символы в начале и конце строки.
 *
 * @param s Входная строка.
 * @return Строка без начальных и конечных пробельных символов.
 */
std::string trim(const std::string& s);

}  // namespace utils

#endif  // STRING_UTILS_H
