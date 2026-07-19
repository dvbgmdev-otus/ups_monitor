/**
 * @file ups_emulator_process.h
 * @brief RAII-обёртка процесса UPS-эмулятора для интеграционных тестов.
 */
#ifndef UPS_EMULATOR_PROCESS_H
#define UPS_EMULATOR_PROCESS_H

#include <sys/types.h>

#include <cstdint>
#include <string>

/**
 * @brief Запускает UPS-эмулятор и гарантирует его остановку.
 *
 * Класс управляет только жизненным циклом процесса. Проверка готовности
 * SNMP-агента выполняется отдельно конкретным интеграционным тестом.
 */
class UpsEmulatorProcess {
public:
    /**
     * @brief Запускает эмулятор с указанными моделью и UDP-портом.
     * @param executablePath Путь к исполняемому файлу эмулятора.
     * @param model Идентификатор модели ИБП.
     * @param port UDP-порт SNMP-агента.
     * @throws std::runtime_error Если процесс не удалось запустить.
     */
    UpsEmulatorProcess(const std::string& executablePath,
                       const std::string& model,
                       uint16_t port);

    UpsEmulatorProcess(const UpsEmulatorProcess&) = delete;
    UpsEmulatorProcess& operator=(const UpsEmulatorProcess&) = delete;

    /**
     * @brief Останавливает запущенный процесс.
     */
    ~UpsEmulatorProcess();

    /**
     * @brief Останавливает эмулятор сигналом SIGINT и ожидает завершения.
     */
    void stop() noexcept;

    /**
     * @brief Возвращает идентификатор процесса эмулятора.
     */
    pid_t pid() const noexcept;

private:
    void start(const std::string& executablePath,
               const std::string& model,
               uint16_t port);

private:
    pid_t m_pid{ -1 };  ///< Идентификатор дочернего процесса.
};

#endif  // UPS_EMULATOR_PROCESS_H
