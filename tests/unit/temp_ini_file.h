/**
 * @file temp_ini_file.h
 * @brief Вспомогательный класс для создания временных INI-файлов в тестах.
 */

#pragma once

#include <string>
#include <vector>

namespace test {

class TempIniFileStorage {
public:
    TempIniFileStorage() = default;
    ~TempIniFileStorage();

    TempIniFileStorage(const TempIniFileStorage&) = delete;
    TempIniFileStorage& operator=(const TempIniFileStorage&) = delete;

    std::string write(const std::string& content);

private:
    std::vector<std::string> m_files;
};

}  // namespace test
