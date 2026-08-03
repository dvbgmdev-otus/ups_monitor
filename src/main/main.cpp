#include <chrono>
#include <csignal>
#include <iostream>
#include <string>
#include <thread>

#include "cli_options.h"
#include "output_formatter.h"
#include "ups_monitor.h"

namespace {

constexpr std::chrono::milliseconds STATE_CHECK_INTERVAL{ 50 };

volatile std::sig_atomic_t stopRequested = 0;

void handleInterrupt(int) { stopRequested = 1; }

}  // namespace

int main(int argc, char* argv[]) {
    const cli::ParseResult result = cli::parseArguments(argc, argv);
    if (!result.ok()) {
        std::cerr << "error: " << result.error << '\n';
        return 1;
    }

    if (result.options.helpRequested) {
        const std::string executableName = argc > 0 && argv[0] != nullptr ? argv[0] : "ups_monitor";
        std::cout << cli::makeHelp(executableName);
        return 0;
    }

    if (std::signal(SIGINT, handleInterrupt) == SIG_ERR) {
        std::cerr << "error: failed to install SIGINT handler\n";
        return 1;
    }

    UpsMonitor monitor;
    ups::ErrorMessage error;
    if (!monitor.init(result.options.ip, result.options.port, error)) {
        std::cerr << "error: " << error << '\n';
        return 1;
    }

    std::cout << output::formatDetectedModel(monitor.modelName(), std::chrono::system_clock::now())
              << std::endl;

    while (!stopRequested) {
        ups::UpsState state;
        if (monitor.tryConsumeState(state)) {
            std::cout << output::formatState(state, std::chrono::system_clock::now()) << std::endl;
        } else {
            std::this_thread::sleep_for(STATE_CHECK_INTERVAL);
        }
    }

    monitor.stop();

    return 0;
}
