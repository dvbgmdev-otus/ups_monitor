/**
 * @file temp_ini_file.cpp
 * @brief Реализация вспомогательного класса для временных INI-файлов.
 */

#include "temp_ini_file.h"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace test {

TempIniFileStorage::~TempIniFileStorage() {
    for (const std::string& file : m_files) {
        std::remove(file.c_str());
    }
}

std::string TempIniFileStorage::write(const std::string& content) {
    const char* tempDir = std::getenv("TMPDIR");
    std::string pathTemplate = tempDir != nullptr && tempDir[0] != '\0' ? tempDir : "/tmp";
    if (pathTemplate.back() != '/') {
        pathTemplate += '/';
    }
    pathTemplate += "ups_monitor_XXXXXX";

    std::vector<char> pathBuffer(pathTemplate.begin(), pathTemplate.end());
    pathBuffer.push_back('\0');

    const int fileDescriptor = mkstemp(pathBuffer.data());
    if (fileDescriptor == -1) {
        throw std::runtime_error("Failed to create temporary INI file");
    }

    const std::string path(pathBuffer.data());
    close(fileDescriptor);

    std::ofstream out(path);
    if (!out.good()) {
        std::remove(path.c_str());
        throw std::runtime_error("Failed to open temporary INI file: " + path);
    }

    out << content;
    out.close();
    if (!out.good()) {
        std::remove(path.c_str());
        throw std::runtime_error("Failed to write temporary INI file: " + path);
    }

    m_files.push_back(path);
    return path;
}

}  // namespace test
