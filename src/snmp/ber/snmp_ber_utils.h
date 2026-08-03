/**
 * @file snmp_ber_utils.h
 * @ingroup snmp_ber
 * @brief Вспомогательные функции ASN.1 BER-кодирования для SNMP.
 */
#ifndef SNMP_BER_UTILS_H
#define SNMP_BER_UTILS_H

#include <string>

#include "snmp_ber_writer.h"
#include "snmp_codec_types.h"  // Oid

namespace snmp {
namespace ber {

/**
 * @brief Кодирует ASN.1 INTEGER.
 * @param w BER-writer выходного буфера.
 * @param value Значение для кодирования.
 */
void encodeInteger(BerWriter& w, int value);

/**
 * @brief Кодирует ASN.1 OCTET STRING.
 * @param w BER-writer выходного буфера.
 * @param str Строка для кодирования.
 */
void encodeOctetString(BerWriter& w, const std::string& str);

/**
 * @brief Кодирует ASN.1 NULL.
 * @param w BER-writer выходного буфера.
 */
void encodeNull(BerWriter& w);

/**
 * @brief Кодирует ASN.1 OBJECT IDENTIFIER.
 * @param w BER-writer выходного буфера.
 * @param oid OID в строковом формате.
 */
void encodeOid(BerWriter& w, const Oid& oid);

}  // namespace ber
}  // namespace snmp

#endif  // SNMP_BER_UTILS_H
