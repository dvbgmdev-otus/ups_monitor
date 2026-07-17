/**
 * @file snmp_client.h
 * @ingroup snmp
 * @brief UDP SNMP-клиент для GET-запросов.
 */
#ifndef SNMP_CLIENT_H
#define SNMP_CLIENT_H

#include <netinet/in.h>

#include <cstdint>
#include <string>
#include <vector>

#include "snmp_client_iface.h"
#include "snmp_codec_types.h"  // SnmpValue, Oid, ErrorMessage

namespace snmp {

/**
 * @brief SNMP client (manager) для GET-запросов.
 *
 * Отвечает за:
 * - формирование SNMP GET
 * - отправку по UDP
 * - приём ответа
 * - декодирование GET-RESPONSE
 *
 * Не содержит логики UPS и интерпретации значений.
 */
class SnmpClient : public ISnmpClient {
public:
    /**
     * @brief Создаёт SNMP-клиент для указанного узла.
     * @param host IPv4-адрес SNMP-агента.
     * @param port UDP-порт SNMP-агента.
     * @param community SNMP community string.
     */
    explicit SnmpClient(const std::string& host,
                        uint16_t port = 161,
                        const std::string& community = "public");

    /**
     * @brief Закрывает UDP-сокет клиента.
     */
    ~SnmpClient() noexcept override;

    /**
     * @brief Выполняет SNMP GET для одного OID.
     * @param oid Запрашиваемый OID.
     * @param out [out] Полученное значение.
     * @param err [out] Текст ошибки, если указатель не равен nullptr.
     * @return true, если значение успешно получено.
     */
    bool get(const Oid& oid, codec::SnmpValue& out, ErrorMessage* err = nullptr) override;

    /**
     * @brief Выполняет SNMP GET для набора OID.
     * @param oids Запрашиваемые OID.
     * @param out [out] Полученные значения.
     * @param err [out] Текст ошибки, если указатель не равен nullptr.
     * @return true, если значения успешно получены.
     */
    bool get(const std::vector<Oid>& oids,
             std::vector<codec::SnmpValue>& out,
             ErrorMessage* err = nullptr) override;

private:
    /**
     * @brief Открывает UDP-сокет при необходимости.
     * @param err [out] Текст ошибки открытия.
     * @return true, если сокет открыт или уже был открыт.
     */
    bool openSocket(ErrorMessage& err);

    /**
     * @brief Закрывает UDP-сокет, если он открыт.
     */
    void closeSocket();

private:
    int m_sock{ -1 };       ///< UDP-сокет клиента.
    sockaddr_in m_addr;     ///< Адрес SNMP-агента.

    std::string m_community;  ///< SNMP community string.
    int m_requestId{ 1 };     ///< Следующий идентификатор SNMP-запроса.
};

}  // namespace snmp

#endif  // SNMP_CLIENT_H
