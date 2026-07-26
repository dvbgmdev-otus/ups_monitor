/**
 * @file fake_snmp_client.h
 * @brief Тестовая реализация интерфейса SNMP-клиента.
 */

#ifndef FAKE_SNMP_CLIENT_H
#define FAKE_SNMP_CLIENT_H

#include <unordered_map>
#include <vector>

#include "snmp_client_iface.h"

namespace test {

/**
 * @class FakeSnmpClient
 * @brief Возвращает заранее заданные результаты SNMP-запросов.
 */
class FakeSnmpClient : public snmp::ISnmpClient {
public:
    /// Заранее заданный ответ на SNMP GET-запрос.
    struct Response {
        bool ok;                       ///< Результат выполнения SNMP GET-запроса.
        snmp::codec::SnmpValue value;  ///< Значение, возвращаемое при успешном запросе.
    };

    void set(const snmp::Oid& oid, const Response& response) { m_table[oid] = response; }

    bool get(const snmp::Oid& oid,
             snmp::codec::SnmpValue& out,
             snmp::ErrorMessage* = nullptr) override {
        const auto it = m_table.find(oid);
        if (it == m_table.end()) {
            return false;
        }

        if (!it->second.ok) {
            return false;
        }

        out = it->second.value;
        return true;
    }

    bool get(const std::vector<snmp::Oid>&,
             std::vector<snmp::codec::SnmpValue>&,
             snmp::ErrorMessage* = nullptr) override {
        return false;  // Групповые SNMP-запросы не поддерживаются.
    }

private:
    std::unordered_map<snmp::Oid, Response> m_table;
};

}  // namespace test

#endif  // FAKE_SNMP_CLIENT_H
