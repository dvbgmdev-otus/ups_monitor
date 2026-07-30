/**
 * @file ups_state_poller.h
 * @ingroup ups
 * @brief Доменный сервис опроса состояния UPS.
 */
#ifndef UPS_STATE_POLLER_H
#define UPS_STATE_POLLER_H

namespace snmp {
class ISnmpClient;
}

namespace ups {

struct UpsState;
class UpsModelSpec;

/**
 * @class UpsStatePoller
 * @brief Доменный сервис опроса состояния UPS.
 *
 * Класс выполняет один цикл опроса UPS по SNMP:
 *  - опрашивает все параметры модели;
 *  - проверяет значения параметров на соответствие спецификации;
 *  - формирует диагностическое поле descr (UpsStateDesc);
 *  - агрегирует итоговый статус UPS (DevState*ProtocolVK).
 *
 * Класс не управляет периодичностью опроса и не хранит состояние
 * между вызовами. Каждый вызов poll() возвращает независимое
 * состояние UPS.
 */
class UpsStatePoller {
public:
    /**
     * @brief Выполнить один цикл опроса UPS и сформировать состояние.
     *
     * @param spec     Спецификация модели UPS.
     * @param client   SNMP-клиент для выполнения запросов.
     * @return Состояние UPS на момент выполнения опроса.
     */
    static UpsState poll(const UpsModelSpec& spec, snmp::ISnmpClient& client);
};

}  // namespace ups

#endif  // UPS_STATE_POLLER_H
