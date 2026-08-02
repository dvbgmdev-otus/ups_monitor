#include <iostream>

#include "cli_options.h"
#include "debug_log.h"

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

    DEBUG_LOG("Application started");

    return 0;
}
