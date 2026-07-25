/**
 * @file ups_model_spec.cpp
 * @ingroup ups
 * @brief Реализация загрузки и валидации спецификации модели UPS.
 */
#include "ups_model_spec.h"

#include <fstream>

#include "fs_utils.h"
#include "string_utils.h"

namespace ups {

bool UpsModelSpec::load(const std::string& path,  // NOLINT(bugprone-easily-swappable-parameters)
                        const std::string& section) {
    m_lastError.clear();
    m_modelName.clear();
    m_modelNameOid.clear();
    m_parameters.clear();

    std::string fullPath = utils::resolvePath(path);
    std::ifstream file(fullPath);
    if (!file.is_open()) {
        m_lastError = "file not found";
        return false;
    }

    bool inSection = false;
    bool sectionFound = false;

    std::string line;
    while (std::getline(file, line)) {
        line = utils::trim(line);

        // ---- пропустить пустые строки и комментарии ----
        // clang-format off
        if (line.empty() ||
            line[0] == '#')
            continue;
        // clang-format on

        // ---- определение секции ----
        // clang-format off
        if (line.front() == '[' &&
            line.back() == ']') {
            std::string sec = line.substr(1, line.size() - 2);
            inSection = (sec == section);
            if (inSection) sectionFound = true;
            continue;
        }
        // clang-format on
        if (!inSection) continue;

        // ---- должен быть знак '=' ----
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        // если дошли до сюда, то есть key=value
        std::string key = utils::trim(line.substr(0, eq));
        std::string value = utils::trim(line.substr(eq + 1));

        // ---- modelName ----
        if (key == "modelName") {
            m_modelName = value;
            continue;
        }

        // ---- <param>.<field> ----
        size_t dot = key.find('.');
        if (dot == std::string::npos) continue;

        // если дошли сюда то есть параметр paramName.field
        std::string paramName = key.substr(0, dot);
        // modelName.* — не UPS-параметр
        if (paramName == "modelName") {
            m_modelNameOid = snmp::Oid(value);
            continue;
        }
        std::string field = key.substr(dot + 1);

        // Регистрируем параметр
        auto& spec = m_parameters[paramName];
        spec.name = paramName;

        if (field == "oid") {
            if (!spec.oid.empty()) {
                m_lastError = "duplicate parameter: ";
                m_lastError += paramName;
                return false;
            }
            spec.oid = value;
        } else if (field == "normal") {
            std::string error;
            if (!parseNormal(value, spec.normal, error)) {
                spec.normal = NormalValueSpec{};
            }
        } else if (field == "bypass") {
            std::string error;
            if (!parseEnumValues(value, spec.bypass, error)) {
                m_lastError = "invalid bypass for parameter ";
                m_lastError += paramName;
                m_lastError += ": ";
                m_lastError += error;
                return false;
            }
        }
    }

    if (!sectionFound) {
        m_lastError = section;
        m_lastError += " section not found or ";
        m_lastError += section;
        m_lastError += " empty";
        return false;
    }

    return validate();
}

const snmp::Oid& UpsModelSpec::modelNameOid() const { return m_modelNameOid; }

const std::string& UpsModelSpec::modelName() const { return m_modelName; }

const std::map<ParamName, UpsParamSpec>& UpsModelSpec::parameters() const { return m_parameters; }

const ErrorMessage& UpsModelSpec::lastError() const { return m_lastError; }

bool UpsModelSpec::parseNormal(const std::string& value, NormalValueSpec& out, ErrorMessage& error) {
    out = NormalValueSpec{};
    error.clear();

    std::string s = utils::trim(value);
    if (s.empty()) {
        error = "normal is empty";
        return false;
    }

    // 1. Попытка range (ЕСЛИ есть ..)
    if (s.find("..") != std::string::npos) {
        uint32_t min = 0, max = 0;
        if (!parseRange(s, min, max, error)) {
            return false;  // ошибка range
        }

        out.isRange = true;
        out.min = min;
        out.max = max;
        return true;
    }

    // 2. Иначе — это enum (даже если одно число)
    if (!parseEnumValues(s, out.values, error)) {
        return false;
    }

    out.isRange = false;
    return true;
}

bool UpsModelSpec::parseRange(const std::string& s,
                              uint32_t& min,
                              uint32_t& max,
                              ErrorMessage& error) {
    error.clear();

    // range обязан содержать ровно один ".."
    size_t dotsPos = s.find("..");
    if (dotsPos == std::string::npos) return false;  // это не range, не ошибка

    if (s.find("..", dotsPos + 2) != std::string::npos) {
        error = "invalid range syntax";
        return false;
    }

    std::string minStr = utils::trim(s.substr(0, dotsPos));
    std::string maxStr = utils::trim(s.substr(dotsPos + 2));

    if (minStr.empty() || maxStr.empty()) {
        error = "invalid range syntax";
        return false;
    }

    try {
        size_t p1 = 0, p2 = 0;
        min = std::stoul(minStr, &p1);
        max = std::stoul(maxStr, &p2);

        // ВАЖНО: проверяем, что строка съедена полностью
        if (p1 != minStr.size() || p2 != maxStr.size()) {
            error = "range contains non-numeric value";
            return false;
        }
    } catch (...) {
        error = "range contains non-numeric value";
        return false;
    }

    if (min > max) {
        error = "range min greater than max";
        return false;
    }

    return true;
}

bool UpsModelSpec::parseEnumValues(const std::string& s,
                                   std::vector<uint32_t>& out,
                                   ErrorMessage& error) {
    out.clear();
    error.clear();

    size_t pos = 0;
    while (pos < s.size()) {
        size_t comma = s.find(',', pos);
        std::string token =
            (comma == std::string::npos) ? s.substr(pos) : s.substr(pos, comma - pos);

        token = utils::trim(token);
        if (token.empty()) {
            error = "enum contains empty value";
            return false;
        }

        try {
            size_t parsed = 0;
            uint32_t value = std::stoul(token, &parsed);

            // ВАЖНО: проверяем, что ВСЯ строка — число
            if (parsed != token.size()) {
                error = "enum contains invalid characters";
                return false;
            }

            out.push_back(value);
        } catch (...) {
            error = "enum contains non-numeric value";
            return false;
        }

        if (comma == std::string::npos) break;

        pos = comma + 1;
    }

    return true;
}

bool UpsModelSpec::validate() {
    m_lastError.clear();

    if (m_modelName.empty()) {
        m_lastError = "modelName missing";
        return false;
    }

    if (m_parameters.empty()) {
        m_lastError = "no parameters defined";
        return false;
    }

    for (const auto& kv : m_parameters) {
        const auto& spec = kv.second;

        if (spec.oid.empty()) {
            m_lastError = "parameter without oid: " + spec.name;
            return false;
        }

        const bool hasNormal = spec.normal.isRange || !spec.normal.values.empty();
        const bool hasBypass = !spec.bypass.empty();
        if (!hasNormal && !hasBypass) {
            m_lastError = "parameter without normal or bypass: " + spec.name;
            return false;
        }
    }

    return true;
}

}  // namespace ups
