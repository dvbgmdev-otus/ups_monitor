/**
 * @file ini_section_reader.cpp
 * @ingroup utils
 * @brief Реализация чтения списка секций INI-файла.
 */
#include "ini_section_reader.h"

#include <algorithm>
#include <fstream>

#include "fs_utils.h"
#include "string_utils.h"

namespace utils {

IniSectionReader::IniSectionReader(const std::string& path) : m_path(path) { scan(); }

void IniSectionReader::scan() {
    m_sections.clear();
    m_lastError.clear();
    m_ok = false;

    std::string fullPath = utils::resolvePath(m_path);

    std::ifstream file(fullPath);
    if (!file) {
        m_lastError = "Cannot open file: " + m_path;
        return;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;

        line = utils::trim(line);

        // ---- пропустить пустые строки и комментарии ----
        // clang-format off
        if (line.empty() || 
            line[0] == '#')
        // clang-format on 
            continue;

        // Строка должна быть вида [SECTION]
        // clang-format off
        if (line.front() == '[' && 
            line.back() == ']')
        // clang-format on
        {
            std::string section = line.substr(1, line.size() - 2);
            section = utils::trim(section);

            if (section.empty()) {
                m_lastError = "Empty section name at line " + std::to_string(lineNumber);
                return;
            }

            // Проверка дубликатов
            if (std::find(m_sections.begin(), m_sections.end(), section) != m_sections.end()) {
                m_lastError =
                    "Duplicate section name: " + section + " at line " + std::to_string(lineNumber);
                return;
            }

            m_sections.push_back(section);
        }
    }

    if (m_sections.empty()) {
        m_lastError = "No sections found in file: " + m_path;
        return;
    }

    // Если дошли до сюда то все хорошо
    m_ok = true;
}

bool IniSectionReader::ok() const { return m_ok; }
const std::vector<std::string>& IniSectionReader::sections() const { return m_sections; }
const std::string& IniSectionReader::lastError() const { return m_lastError; }

}  // namespace utils
