/**
 * @file ups_emulator_process.cpp
 * @brief Реализация RAII-обёртки процесса UPS-эмулятора.
 */
#include "ups_emulator_process.h"

#include <limits.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace {

std::string resolveExecutablePath(const std::string& path) {
    char resolvedPath[PATH_MAX];
    if (::realpath(path.c_str(), resolvedPath) == nullptr) {
        throw std::runtime_error("Failed to resolve UPS emulator path: " +
                                 std::string(std::strerror(errno)));
    }
    return resolvedPath;
}

std::string parentDirectory(const std::string& path) {
    const std::string::size_type separator = path.find_last_of('/');
    if (separator == std::string::npos) {
        return ".";
    }
    if (separator == 0) {
        return "/";
    }
    return path.substr(0, separator);
}

}  // namespace

UpsEmulatorProcess::UpsEmulatorProcess(const std::string& executablePath,
                                       const std::string& model,
                                       uint16_t port) {
    start(executablePath, model, port);
}

UpsEmulatorProcess::~UpsEmulatorProcess() { stop(); }

pid_t UpsEmulatorProcess::pid() const noexcept { return m_pid; }

void UpsEmulatorProcess::start(const std::string& executablePath,
                               const std::string& model,
                               uint16_t port) {
    const std::string resolvedPath = resolveExecutablePath(executablePath);
    const std::string workingDirectory = parentDirectory(resolvedPath);
    const std::string portArgument = std::to_string(port);

    m_pid = ::fork();
    if (m_pid < 0) {
        throw std::runtime_error("Failed to fork UPS emulator process");
    }

    if (m_pid == 0) {
        if (::chdir(workingDirectory.c_str()) != 0) {
            ::perror("Failed to change UPS emulator working directory");
            ::_exit(127);
        }

        ::execl(resolvedPath.c_str(),
                "ups_emulator",
                "--model",
                model.c_str(),
                "--port",
                portArgument.c_str(),
                static_cast<char*>(nullptr));

        ::perror("Failed to start UPS emulator");
        ::_exit(127);
    }
}

void UpsEmulatorProcess::stop() noexcept {
    if (m_pid <= 0) {
        return;
    }

    ::kill(m_pid, SIGINT);

    int status = 0;
    while (::waitpid(m_pid, &status, 0) < 0 && errno == EINTR) {
    }

    m_pid = -1;
}
