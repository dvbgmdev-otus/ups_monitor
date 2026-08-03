/**
 * @file ups_state_poller.h
 * @ingroup ups
 * @brief Доменный сервис опроса состояния ИБП.
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
 * @brief Доменный сервис опроса состояния ИБП.
 *
 * Класс выполняет один цикл опроса ИБП по SNMP:
 *  - опрашивает все параметры модели;
 *  - проверяет значения параметров на соответствие спецификации;
 *  - формирует битовую маску отклонений;
 *  - агрегирует итоговый статус ИБП.
 *
 * Класс не управляет периодичностью опроса и не хранит состояние
 * между вызовами. Каждый вызов poll() возвращает независимое
 * состояние ИБП.
 */
class UpsStatePoller {
public:
    /**
     * @brief Выполнить один цикл опроса ИБП и сформировать состояние.
     *
     * @param spec     Спецификация модели ИБП.
     * @param client   SNMP-клиент для выполнения запросов.
     * @return Состояние ИБП на момент выполнения опроса.
     */
    static UpsState poll(const UpsModelSpec& spec, snmp::ISnmpClient& client);
};

}  // namespace ups

#endif  // UPS_STATE_POLLER_H
