/**
 * @file cli_options.h
 * @brief Разбор аргументов командной строки приложения.
 */
#ifndef CLI_OPTIONS_H
#define CLI_OPTIONS_H

#include <cstdint>
#include <string>

namespace cli {

/**
 * @brief Параметры запуска приложения.
 */
struct Options {
    std::string ip{ "127.0.0.1" };
    uint16_t port{ 161 };
    bool helpRequested{ false };
};

/**
 * @brief Результат разбора аргументов командной строки.
 */
struct ParseResult {
    Options options;
    std::string error;

    bool ok() const noexcept { return error.empty(); }
};

/**
 * @brief Разбирает и проверяет аргументы командной строки.
 * @param argc Количество аргументов.
 * @param argv Массив аргументов.
 * @return Параметры запуска либо описание ошибки.
 */
ParseResult parseArguments(int argc, const char* const argv[]);

/**
 * @brief Формирует справку по использованию приложения.
 * @param executableName Имя исполняемого файла.
 * @return Текст справки.
 */
std::string makeHelp(const std::string& executableName);

}  // namespace cli

#endif  // CLI_OPTIONS_H
