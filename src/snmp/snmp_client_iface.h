/**
 * @file snmp_client_iface.h
 * @ingroup snmp
 * @brief Интерфейс SNMP-клиента для доменной логики UPS.
 */
#ifndef SNMP_CLIENT_IFACE_H
#define SNMP_CLIENT_IFACE_H

#include <vector>

#include "snmp_codec_types.h"  // SnmpValue, Oid, ErrorMessage

namespace snmp {

/**
 * @brief Интерфейс SNMP-клиента.
 *
 * Используется доменной логикой UPS для выполнения SNMP-запросов
 * без привязки к конкретной реализации (UDP, эмулятор, mock).
 */
class ISnmpClient {
public:
    /**
     * @brief Деструктор.
     */
    virtual ~ISnmpClient() noexcept = default;

    /**
     * @brief Выполняет SNMP GET для одного OID.
     * @param oid Запрашиваемый OID.
     * @param out [out] Полученное значение.
     * @param err [out] Текст ошибки, если указатель не равен nullptr.
     * @return true, если значение успешно получено.
     */
    virtual bool get(const Oid& oid, codec::SnmpValue& out, ErrorMessage* err = nullptr) = 0;

    /**
     * @brief Выполняет SNMP GET для набора OID.
     * @param oids Запрашиваемые OID.
     * @param out [out] Полученные значения.
     * @param err [out] Текст ошибки, если указатель не равен nullptr.
     * @return true, если значения успешно получены.
     */
    virtual bool get(const std::vector<Oid>& oids,
                     std::vector<snmp::codec::SnmpValue>& out,
                     ErrorMessage* err = nullptr) = 0;
};

}  // namespace snmp

#endif  // SNMP_CLIENT_IFACE_H
