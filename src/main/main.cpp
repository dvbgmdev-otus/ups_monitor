#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include "cli_options.h"
#include "output_formatter.h"
#include "ups_monitor.h"

namespace {

// Интервал проверки наличия нового состояния ИБП.
constexpr std::chrono::milliseconds STATE_CHECK_INTERVAL{ 50 };

// Флаг запроса штатного завершения приложения.
volatile std::sig_atomic_t stopRequested = 0;

// Обработчик SIGINT устанавливает флаг завершения для основного цикла.
void handleInterrupt(int) { stopRequested = 1; }

}  // namespace

int main(int argc, char* argv[]) {
    // Этап 1. Разбор и проверка аргументов командной строки.
    const cli::ParseResult result = cli::parseArguments(argc, argv);
    if (!result.ok()) {
        std::cerr << "error: " << result.error << '\n';
        return 1;
    }

    // Этап 2. Вывод справки без запуска мониторинга.
    if (result.options.helpRequested) {
        const std::string executableName = argc > 0 && argv[0] != nullptr ? argv[0] : "ups_monitor";
        std::cout << cli::makeHelp(executableName);
        return 0;
    }

    // Этап 3. Установка обработчика штатного завершения по Ctrl+C.
    if (std::signal(SIGINT, handleInterrupt) == SIG_ERR) {
        std::cerr << "error: failed to install SIGINT handler\n";
        return 1;
    }

    // Этап 4. Инициализация монитора и определение модели ИБП.
    UpsMonitor monitor;
    ups::ErrorMessage error;
    if (!monitor.init(result.options.ip, result.options.port, error)) {
        std::cerr << "error: " << error << '\n';
        return 1;
    }

    // Этап 5. Вывод названия обнаруженной модели.
    std::cout << output::formatDetectedModel(monitor.modelName(), std::chrono::system_clock::now())
              << std::endl;

    // Этап 6. Получение и вывод результатов фонового опроса.
    while (!stopRequested) {
        ups::UpsState state;
        if (monitor.tryConsumeState(state)) {
            std::cout << output::formatState(state, std::chrono::system_clock::now()) << std::endl;
        } else {
            std::this_thread::sleep_for(STATE_CHECK_INTERVAL);
        }
    }

    // Этап 7. Остановка опроса и освобождение ресурсов.
    monitor.stop();

    return 0;
}
