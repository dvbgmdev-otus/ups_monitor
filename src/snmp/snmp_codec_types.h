/**
 * @file snmp_codec_types.h
 * @ingroup snmp
 * @brief Базовые типы SNMP-кодека.
 */
#ifndef SNMP_CODEC_TYPES_H
#define SNMP_CODEC_TYPES_H

#include <cstdint>
#include <string>

namespace snmp {

using Oid = std::string;           ///< SNMP OID в строковом формате.
using ErrorMessage = std::string;  ///< Текст ошибки SNMP-операции.

namespace codec {

/**
 * @brief Версия протокола SNMP.
 */
enum class SnmpVersion : uint8_t {
    V_1 = 0,  ///< SNMP v1
    V_2C = 1  ///< SNMP v2c
};

/**
 * @brief Тип значения SNMP VarBind.
 */
struct SnmpValue {
    Oid oid;  ///< OID значения из VarBind.

    /**
     * @brief Тип значения SNMP VarBind.
     */
    enum class Type : std::uint8_t {
        Integer,  ///< Целочисленное значение.
        String,   ///< Строковое значение.
        Null      ///< Значение отсутствует.
    };

    Type type{ Type::Null };  ///< Тип значения.
    int intValue{ 0 };        ///< Целочисленное значение для Type::Integer.
    std::string strValue;     ///< Строковое значение для Type::String.
};

}  // namespace codec
}  // namespace snmp

#endif  // SNMP_CODEC_TYPES_H
