/**
 * @file cli_options.cpp
 * @brief Реализация разбора аргументов командной строки приложения.
 */
#include "cli_options.h"

#include <arpa/inet.h>

#include <sstream>

namespace cli {
namespace {

/**
 * @brief Проверяет, начинается ли аргумент с префикса длинной опции.
 * @param value Значение аргумента командной строки.
 * @return true, если аргумент начинается с `--`.
 */
bool isOption(const char* value) { return value != nullptr && value[0] == '-' && value[1] == '-'; }

/**
 * @brief Проверяет строку на соответствие формату IPv4-адреса.
 * @param value Проверяемая строка.
 * @return true, если строка содержит корректный IPv4-адрес.
 */
bool isValidIpv4(const std::string& value) {
    in_addr address;
    return inet_pton(AF_INET, value.c_str(), &address) == 1;
}

/**
 * @brief Разбирает и проверяет номер UDP-порта.
 * @param value Строковое представление порта.
 * @param port [out] Полученный номер порта.
 * @return true, если строка содержит целое число в диапазоне от 1 до 65535.
 */
bool parsePort(const std::string& value, uint16_t& port) {
    if (value.empty()) return false;

    uint32_t parsed = 0;
    for (std::string::const_iterator it = value.begin(); it != value.end(); ++it) {
        if (*it < '0' || *it > '9') return false;

        parsed = parsed * 10 + static_cast<uint32_t>(*it - '0');
        if (parsed > 65535) return false;
    }

    if (parsed == 0) return false;

    port = static_cast<uint16_t>(parsed);
    return true;
}

/**
 * @brief Проверяет наличие значения после опции.
 * @param index Индекс опции в массиве аргументов.
 * @param argc Количество аргументов.
 * @param argv Массив аргументов.
 * @return true, если следующий аргумент существует и не является длинной опцией.
 */
bool hasValue(int index, int argc, const char* const argv[]) {
    return index + 1 < argc && argv[index + 1] != nullptr && !isOption(argv[index + 1]);
}

}  // namespace

ParseResult parseArguments(int argc, const char* const argv[]) {
    ParseResult result;
    bool ipSpecified = false;
    bool portSpecified = false;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index] == nullptr ? "" : argv[index];

        if (argument == "--help") {
            if (argc != 2) {
                result.error = "--help cannot be combined with other arguments";
                return result;
            }
            result.options.helpRequested = true;
            return result;
        }

        if (argument == "--ip") {
            if (ipSpecified) {
                result.error = "argument --ip specified more than once";
                return result;
            }
            if (!hasValue(index, argc, argv)) {
                result.error = "missing value for --ip";
                return result;
            }

            const std::string value = argv[++index];
            if (!isValidIpv4(value)) {
                result.error = "invalid IPv4 address: " + value;
                return result;
            }

            result.options.ip = value;
            ipSpecified = true;
            continue;
        }

        if (argument == "--port") {
            if (portSpecified) {
                result.error = "argument --port specified more than once";
                return result;
            }
            if (!hasValue(index, argc, argv)) {
                result.error = "missing value for --port";
                return result;
            }

            const std::string value = argv[++index];
            if (!parsePort(value, result.options.port)) {
                result.error = "port must be an integer in range 1..65535: " + value;
                return result;
            }

            portSpecified = true;
            continue;
        }

        result.error = "unknown argument: " + argument;
        return result;
    }

    return result;
}

std::string makeHelp(const std::string& executableName) {
    const std::string name = executableName.empty() ? "ups_monitor" : executableName;
    std::ostringstream help;
    help << "Usage:\n"
         << "  " << name << " [--ip <IPv4>] [--port <number>]\n"
         << "  " << name << " --help\n\n"
         << "Options:\n"
         << "  --ip <IPv4>    UPS IPv4 address (default: 127.0.0.1)\n"
         << "  --port <number> SNMP UDP port, 1..65535 (default: 161)\n"
         << "  --help         Show this help and exit\n";
    return help.str();
}

}  // namespace cli
