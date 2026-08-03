/**
 * @file snmp_ber_utils.cpp
 * @ingroup snmp_ber
 * @brief Реализация вспомогательных функций BER-кодирования.
 */
#include "snmp_ber_utils.h"

#include <vector>

#include "snmp_ber_tags.h"
#include "snmp_codec_types.h"  // Oid, ErrorMessage

namespace snmp {
namespace ber {

void encodeInteger(BerWriter& w, int value) {
    // Шаг 1: представляем число в виде 4 bytes big-endian
    uint8_t raw[4];
    raw[0] = (value >> 24) & 0xFF;
    raw[1] = (value >> 16) & 0xFF;
    raw[2] = (value >> 8) & 0xFF;
    raw[3] = value & 0xFF;

    // Шаг 2: удаляем лишние leading bytes
    int start = 0;
    while (start < 3) {
        // положительное число - удаляем лишние 00, если следующий байт < 0x80
        if (raw[start] == 0x00 && (raw[start + 1] & 0x80) == 0) {
            start++;
            continue;
        }
        // отрицательное число - удаляем лишние FF, если следующий байт >= 0x80
        if (raw[start] == 0xFF && (raw[start + 1] & 0x80) == 0x80) {
            start++;
            continue;
        }
        break;
    }
    const uint8_t* encoded = raw + start;
    size_t n = 4 - start;

    // Шаг 3: ASN.1 INTEGER
    w.putTag(TAG_INTEGER);
    w.putLength(n);
    w.putBytes(encoded, n);
}

void encodeOctetString(BerWriter& w, const std::string& str) {
    w.putTag(TAG_OCTETSTRING);
    w.putLength(str.size());
    if (!str.empty()) {
        w.putBytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }
}

void encodeNull(BerWriter& w) {
    w.putTag(TAG_NULL);  // 0x05
    w.putLength(0);      // длина = 0
}

void encodeOid(BerWriter& w, const Oid& oid) {
    // ------------------------------------------------------------
    // 1. Парсим OID "1.3.6.1.4.1.9999.1" -> [1,3,6,1,4,1,9999,1]
    // ------------------------------------------------------------
    std::vector<uint32_t> parts;
    size_t pos = 0;
    while (pos < oid.size()) {
        size_t dot = oid.find('.', pos);
        std::string token =
            (dot == std::string::npos) ? oid.substr(pos) : oid.substr(pos, dot - pos);
        parts.push_back(std::stoul(token));
        if (dot == std::string::npos) break;
        pos = dot + 1;
    }
    if (parts.size() < 2) {
        w.putTag(TAG_OID);
        w.putLength(0);
        return;
    }

    // ------------------------------------------------------------
    // 2. ASN.1 правило: первый байт = 40 * X + Y
    // ------------------------------------------------------------
    uint8_t first = uint8_t(parts[0] * 40 + parts[1]);
    std::vector<uint8_t> enc;
    enc.push_back(first);

    // ------------------------------------------------------------
    // Функция минимального BER base-128 кодирования
    // ------------------------------------------------------------
    auto encodeBase128 = [&](uint32_t value) {
        uint8_t tmp[10];
        int count = 0;
        do {
            tmp[count++] = value & 0x7F;
            value >>= 7;
        } while (value > 0);
        for (int i = count - 1; i >= 0; --i) {
            uint8_t b = tmp[i];
            if (i != 0) b |= 0x80;  // continuation bit
            enc.push_back(b);
        }
    };

    // ------------------------------------------------------------
    // 3. Кодируем все компоненты, начиная с третьего
    // ------------------------------------------------------------
    for (size_t i = 2; i < parts.size(); ++i) {
        encodeBase128(parts[i]);
    }

    // ------------------------------------------------------------
    // 4. Собираем TLV (TAG_OID | LENGTH | VALUE)
    // ------------------------------------------------------------
    w.putTag(TAG_OID);
    w.putLength(enc.size());
    w.putBytes(enc.data(), enc.size());
}

}  // namespace ber
}  // namespace snmp
