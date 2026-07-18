/**
 * @file snmp_codec.h
 * @ingroup snmp
 * @brief Кодирование SNMP GET-request и декодирование GET-response.
 */
#ifndef SNMP_CODEC_H
#define SNMP_CODEC_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "snmp_codec_types.h"  // SnmpVersion, SnmpValue, Oid, ErrorMessage

namespace snmp {
namespace ber {
class BerWriter;
}

namespace codec {

/**
 * @brief SNMP GET-request (логическое представление).
 *
 * Используется кодеком при формировании GET-запроса клиентом.
 */
struct SnmpGetRequest {
    int requestId{ 0 };                        ///< Request-ID
    SnmpVersion version{ SnmpVersion::V_2C };  ///< Версия SNMP
    std::string community{ "public" };         ///< Community string
    std::vector<Oid> oids;                     ///< OID из VarBindList
};

/**
 * @brief SNMP codec (encoder / decoder).
 *
 * Отвечает ТОЛЬКО за:
 *  - формирование SNMP GET-request
 *  - разбор SNMP GET-response
 *
 * НЕ:
 *  - работает с сокетами
 *  - содержит логику клиента или агента
 *  - знает что-либо про UPS, модели или хранилища
 */
class SnmpCodec {
public:
    /**
     * @brief Формирует SNMP GET-request.
     *
     * @param req  Логическое описание запроса
     * @param out  Выходной BER-буфер
     */
    static void encodeGetRequest(const SnmpGetRequest& req, std::vector<uint8_t>& out);

    /**
     * @brief Разбирает (декодирует) SNMP GET-response.
     *
     * @param data Входной буфер
     * @param size Размер буфера
     * @param expectedRequestId Ожидаемый идентификатор запроса.
     * @param out [out] Полученные значения.
     * @param err [out] Текст ошибки
     * @return true, если ответ успешно декодирован.
     */
    static bool decodeGetResponse(const uint8_t* data,
                                  size_t size,
                                  int expectedRequestId,
                                  std::vector<SnmpValue>& out,
                                  ErrorMessage& err);

    /**
     * @brief Разбирает GET-response и проверяет версию протокола.
     *
     * @param data Входной буфер
     * @param size Размер буфера
     * @param expectedRequestId Ожидаемый идентификатор запроса.
     * @param expectedVersion Ожидаемая версия протокола.
     * @param out [out] Полученные значения.
     * @param err [out] Текст ошибки
     * @return true, если ответ успешно декодирован.
     */
    static bool decodeGetResponse(const uint8_t* data,
                                  size_t size,
                                  int expectedRequestId,
                                  SnmpVersion expectedVersion,
                                  std::vector<SnmpValue>& out,
                                  ErrorMessage& err);

private:
    static bool decodeGetResponseImpl(const uint8_t* data,
                                      size_t size,
                                      int expectedRequestId,
                                      const SnmpVersion* expectedVersion,
                                      std::vector<SnmpValue>& out,
                                      ErrorMessage& err);

    /**
     * @brief Кодирует один VarBind для GET-request (encode helper).
     *
     * Структура:
     *   VarBind ::= SEQUENCE {
     *       name  OBJECT IDENTIFIER,
     *       value NULL
     *   }
     *
     * @param w BER-writer выходного буфера.
     * @param oid OID для кодирования.
     */
    static void encodeGetVarBind(ber::BerWriter& w, const Oid& oid);

    /**
     * @brief Разбирает (декодирует) один VarBind из GET-response (decode helper).
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param out [out] Прочитанное значение VarBind.
     * @param err [out] Текст ошибки декодирования.
     * @return true, если VarBind успешно декодирован.
     */
    static bool decodeVarBind(const uint8_t*& p,
                              const uint8_t* end,
                              SnmpValue& out,
                              ErrorMessage& err);

    /**
     * @brief Разбирает (декодирует) VarBindList из GET-response (decode helper).
     * @param p [in/out] Текущая позиция чтения.
     * @param end Конец входного буфера.
     * @param out [out] Прочитанные значения VarBind.
     * @param err [out] Текст ошибки декодирования.
     * @return true, если VarBindList успешно декодирован.
     */
    static bool decodeVarBindList(const uint8_t*& p,
                                  const uint8_t* end,
                                  std::vector<SnmpValue>& out,
                                  ErrorMessage& err);
};

}  // namespace codec
}  // namespace snmp

#endif  // SNMP_CODEC_H
