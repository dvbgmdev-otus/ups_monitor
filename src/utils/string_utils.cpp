/**
 * @file string_utils.cpp
 * @ingroup utils
 * @brief Реализация утилит для работы со строками.
 */
#include "string_utils.h"

namespace utils {

std::string trim(const std::string& s) {
    const char* WS = " \t\r\n";

    size_t start = s.find_first_not_of(WS);
    if (start == std::string::npos) return "";

    size_t end = s.find_last_not_of(WS);
    return s.substr(start, end - start + 1);
}

}  // namespace utils
