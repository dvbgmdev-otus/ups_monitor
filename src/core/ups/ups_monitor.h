/**
 * @file ups_monitor.h
 * @ingroup ups
 * @brief Мониторинг состояния ИБП.
 */
#ifndef UPS_MONITOR_H
#define UPS_MONITOR_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "ups_model_spec.h"
#include "ups_state_buffer.h"

namespace snmp {
class ISnmpClient;
}

/**
 * @brief Монитор состояния ИБП по SNMP.
 *
 * Класс управляет SNMP-клиентом, периодическим опросом ИБП и хранением
 * последнего агрегированного состояния.
 */
class UpsMonitor {
public:
    /**
     * @brief Создаёт монитор состояния ИБП.
     */
    UpsMonitor();

    /**
     * @brief Останавливает поток опроса ИБП.
     */
    virtual ~UpsMonitor();

    UpsMonitor(const UpsMonitor&) = delete;
    UpsMonitor& operator=(const UpsMonitor&) = delete;
    UpsMonitor(UpsMonitor&&) = delete;
    UpsMonitor& operator=(UpsMonitor&&) = delete;

    /**
     * @brief Инициализирует монитор ИБП и запускает фоновый опрос.
     * @param ip IP-адрес ИБП.
     * @param port UDP-порт SNMP-агента.
     * @param error [out] Текст ошибки инициализации.
     * @return true при успешной инициализации.
     * @warning Повторный вызов после успешной инициализации запрещён.
     */
    bool init(const std::string& ip, uint16_t port, ups::ErrorMessage& error);

    /**
     * @brief Останавливает фоновый опрос ИБП и ожидает завершения потока.
     */
    void stop();

    /**
     * @brief Пытается получить последнее непотреблённое состояние ИБП.
     * @param state [out] Полученное состояние ИБП.
     * @return true, если новое состояние доступно.
     */
    bool tryConsumeState(ups::UpsState& state);

    /**
     * @brief Возвращает имя обнаруженной модели ИБП.
     * @return Имя модели или пустая строка, если модель ещё не определена.
     */
    const std::string& modelName() const;

protected:
    /**
     * @brief Создаёт SNMP-клиент для указанного IP-адреса.
     * @param ip IP-адрес ИБП.
     * @param port UDP-порт SNMP-агента.
     * @return Экземпляр SNMP-клиента.
     */
    virtual std::unique_ptr<snmp::ISnmpClient> createSnmpClient(const std::string& ip,
                                                                uint16_t port);

    ups::UpsStateBuffer m_stateBuffer;  ///< Хранитель состояния ИБП.

private:
    /**
     * @brief Выполняет периодический опрос ИБП в рабочем потоке.
     */
    void pollLoop();

    std::unique_ptr<snmp::ISnmpClient> m_snmp;  ///< SNMP-клиент для опроса ИБП.

    ups::UpsModelSpec m_modelSpec;         ///< Спецификация обнаруженной модели ИБП.
    std::thread m_thread;                  ///< Рабочий поток опроса ИБП.
    std::condition_variable m_waitCondition;  ///< Условие прерывания ожидания.
    std::mutex m_waitMutex;                   ///< Мьютекс ожидания следующего опроса.
    bool m_initialized{ false };           ///< Признак успешной инициализации.
    std::atomic<bool> m_running{ false };  ///< Признак работы потока опроса.
};

#endif  // UPS_MONITOR_H
