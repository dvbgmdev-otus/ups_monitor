/**
 * @file snmp_ber_reader.h
 * @ingroup snmp_ber
 * @brief Низкоуровневое чтение ASN.1 BER-структур для SNMP.
 */
#ifndef SNMP_BER_READER_H
#define SNMP_BER_READER_H

#include <cstddef>
#include <cstdint>
#include <string>

#include "snmp_ber_tags.h"
#include "snmp_codec_types.h"  // Oid, ErrorMessage

namespace snmp {
namespace ber {

/**
 * @brief Универсальный ASN.1 BER reader.
 *
 * Содержит низкоуровневую логику декодирования ASN.1 BER
 */
class BerReader {
public:
    /**
     * @brief Читает ASN.1 тег и длину значения.
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param expectedTag Ожидаемый ASN.1 тег.
     * @param outLen [out] Длина значения в байтах.
     * @param err [out] Текст ошибки чтения.
     * @return true, если тег и длина успешно прочитаны.
     */
    static bool readTagAndLength(const uint8_t*& p,
                                 const uint8_t* end,
                                 uint8_t expectedTag,
                                 size_t& outLen,
                                 ErrorMessage& err);

    /**
     * @brief Читает ASN.1 SEQUENCE и возвращает границу её содержимого.
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param seqEnd [out] Конец содержимого SEQUENCE.
     * @param err [out] Текст ошибки чтения.
     * @return true, если SEQUENCE успешно прочитана.
     */
    static bool readSequence(const uint8_t*& p,
                             const uint8_t* end,
                             const uint8_t*& seqEnd,
                             ErrorMessage& err);

    /**
     * @brief Читает ASN.1 INTEGER.
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param outValue [out] Прочитанное значение.
     * @param err [out] Текст ошибки чтения.
     * @return true, если значение успешно прочитано.
     */
    static bool readInteger(const uint8_t*& p,
                            const uint8_t* end,
                            int& outValue,
                            ErrorMessage& err);

    /**
     * @brief Читает целочисленное значение с указанным ASN.1 тегом.
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param expectedTag Ожидаемый ASN.1 тег.
     * @param outValue [out] Прочитанное значение.
     * @param err [out] Текст ошибки чтения.
     * @return true, если значение успешно прочитано.
     */
    static bool readIntegerWithTag(const uint8_t*& p,
                                   const uint8_t* end,
                                   uint8_t expectedTag,
                                   int& outValue,
                                   ErrorMessage& err);

    /**
     * @brief Читает ASN.1 OCTET STRING.
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param outStr [out] Прочитанная строка.
     * @param err [out] Текст ошибки чтения.
     * @return true, если строка успешно прочитана.
     */
    static bool readOctetString(const uint8_t*& p,
                                const uint8_t* end,
                                std::string& outStr,
                                ErrorMessage& err);

    /**
     * @brief Читает ASN.1 OBJECT IDENTIFIER.
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param outOid [out] Прочитанный OID.
     * @param err [out] Текст ошибки чтения.
     * @return true, если OID успешно прочитан.
     */
    static bool readOid(const uint8_t*& p, const uint8_t* end, Oid& outOid, ErrorMessage& err);

    /**
     * @brief Пропускает ASN.1 TLV элемент любого типа.
     *
     * Используется, когда значение не нужно интерпретировать,
     * но необходимо корректно продвинуть указатель.
     *
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param err [out] Текст ошибки чтения.
     * @return true, если элемент успешно пропущен.
     */
    static bool skipValue(const uint8_t*& p, const uint8_t* end, ErrorMessage& err);
};

}  // namespace ber
}  // namespace snmp

#endif  // SNMP_BER_READER_H
