/**
 * @file snmp_ready_waiter.h
 * @brief Ожидание готовности SNMP-агента в интеграционных тестах.
 */
#ifndef SNMP_READY_WAITER_H
#define SNMP_READY_WAITER_H

#include <chrono>

#include "snmp_client_iface.h"

/**
 * @brief Ожидает успешного ответа SNMP-агента.
 *
 * Между неуспешными попытками выдерживается заданная пауза. Сам SNMP-запрос
 * может дополнительно ожидать ответ в пределах тайм-аута клиента.
 *
 * @param client SNMP-клиент, настроенный на проверяемый агент.
 * @param oid OID, доступный выбранной модели эмулятора.
 * @param attempts Максимальное количество запросов.
 * @param retryInterval Пауза между запросами.
 * @param err [out] Последняя ошибка SNMP-запроса.
 * @return true после первого успешного ответа.
 */
bool waitUntilSnmpReady(snmp::ISnmpClient& client,
                        const snmp::Oid& oid,
                        unsigned int attempts,
                        std::chrono::milliseconds retryInterval,
                        snmp::ErrorMessage* err = nullptr);

#endif  // SNMP_READY_WAITER_H
