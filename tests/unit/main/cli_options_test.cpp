/**
 * @file cli_options_test.cpp
 * @brief Unit-тесты разбора аргументов командной строки.
 */
#include "cli_options.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <string>

namespace {

cli::ParseResult parse(std::initializer_list<const char*> arguments) {
    return cli::parseArguments(static_cast<int>(arguments.size()), arguments.begin());
}

}  // namespace

#if (1)  // Значения по умолчанию

// Test 1.1: Запуск без опций возвращает значения по умолчанию
TEST(CliOptionsTest, Defaults_NoOptions_ReturnsDefaultEndpoint) {
    const cli::ParseResult result = parse({ "ups_monitor" });
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.options.ip, "127.0.0.1");
    EXPECT_EQ(result.options.port, 161);
    EXPECT_FALSE(result.options.helpRequested);
}

#endif

#if (1)  // Корректные параметры подключения

// Test 2.1: IPv4-адрес и порт заменяют значения по умолчанию
TEST(CliOptionsTest, Endpoint_ValidIpAndPort_ReturnsSpecifiedValues) {
    const cli::ParseResult result =
        parse({ "ups_monitor", "--ip", "192.168.1.10", "--port", "1161" });
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.options.ip, "192.168.1.10");
    EXPECT_EQ(result.options.port, 1161);
}

// Test 2.2: Параметры разрешены в обратном порядке
TEST(CliOptionsTest, Endpoint_ReversedOptionOrder_ReturnsSpecifiedValues) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "65535", "--ip", "10.0.0.1" });
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.options.ip, "10.0.0.1");
    EXPECT_EQ(result.options.port, 65535);
}

#endif

#if (1)  // Справка

// Test 3.1: Единственная опция справки успешно распознаётся
TEST(CliOptionsTest, Help_OnlyArgument_RequestsHelp) {
    const cli::ParseResult result = parse({ "ups_monitor", "--help" });
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_TRUE(result.options.helpRequested);
}

// Test 3.2: Справка содержит имя программы и доступные опции
TEST(CliOptionsTest, HelpText_ExecutableName_ContainsUsageAndOptions) {
    const std::string help = cli::makeHelp("monitor");
    EXPECT_NE(help.find("monitor [--ip <IPv4>] [--port <number>]"), std::string::npos);
    EXPECT_NE(help.find("--help"), std::string::npos);
}

// Test 3.3: Справку нельзя сочетать с другими параметрами
TEST(CliOptionsTest, Help_WithOtherArgument_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--help", "--port", "161" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "--help cannot be combined with other arguments");
}

#endif

#if (1)  // Ошибки структуры аргументов

// Test 4.1: Неизвестный аргумент отклоняется
TEST(CliOptionsTest, Structure_UnknownArgument_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--unknown" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "unknown argument: --unknown");
}

// Test 4.2: Отсутствующее значение IPv4-адреса обнаруживается
TEST(CliOptionsTest, Structure_IpWithoutValue_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--ip" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "missing value for --ip");
}

// Test 4.3: Отсутствующее значение порта обнаруживается
TEST(CliOptionsTest, Structure_PortWithoutValue_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "missing value for --port");
}

// Test 4.4: Следующая опция не принимается как значение порта
TEST(CliOptionsTest, Structure_PortFollowedByOption_ReturnsMissingValueError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "--ip", "10.0.0.1" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "missing value for --port");
}

// Test 4.5: Повторение IPv4-опции отклоняется
TEST(CliOptionsTest, Structure_DuplicateIp_ReturnsError) {
    const cli::ParseResult result =
        parse({ "ups_monitor", "--ip", "10.0.0.1", "--ip", "10.0.0.2" });

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "argument --ip specified more than once");
}

// Test 4.6: Повторение опции порта отклоняется
TEST(CliOptionsTest, Structure_DuplicatePort_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "161", "--port", "1161" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "argument --port specified more than once");
}

#endif

#if (1)  // Проверка IPv4-адреса

// Test 5.1: Адрес с октетом вне диапазона отклоняется
TEST(CliOptionsTest, Ipv4_OctetAbove255_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--ip", "192.168.1.256" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "invalid IPv4 address: 192.168.1.256");
}

// Test 5.2: DNS-имя не принимается вместо IPv4-адреса
TEST(CliOptionsTest, Ipv4_HostName_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--ip", "localhost" });
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.error, "invalid IPv4 address: localhost");
}

#endif

#if (1)  // Проверка UDP-порта

// Test 6.1: Минимальный порт принимается
TEST(CliOptionsTest, Port_MinimumValue_ReturnsPort) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "1" });
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.options.port, 1);
}

// Test 6.2: Нулевой порт отклоняется
TEST(CliOptionsTest, Port_Zero_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "0" });
    EXPECT_FALSE(result.ok());
}

// Test 6.3: Порт выше допустимого диапазона отклоняется
TEST(CliOptionsTest, Port_AboveMaximum_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "65536" });
    EXPECT_FALSE(result.ok());
}

// Test 6.4: Порт с нечисловым суффиксом отклоняется
TEST(CliOptionsTest, Port_NumericPrefixWithSuffix_ReturnsError) {
    const cli::ParseResult result = parse({ "ups_monitor", "--port", "161udp" });
    EXPECT_FALSE(result.ok());
}

// Test 6.5: Очень большое число не приводит к переполнению
TEST(CliOptionsTest, Port_VeryLargeNumber_ReturnsError) {
    const cli::ParseResult result =
        parse({ "ups_monitor", "--port", "999999999999999999999999999999999999" });
    EXPECT_FALSE(result.ok());
}

#endif
