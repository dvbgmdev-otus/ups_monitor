/**
 * @file snmp_ber_reader.cpp
 * @ingroup snmp_ber
 * @brief Реализация чтения ASN.1 BER-структур для SNMP.
 */
#include "snmp_ber_reader.h"

#include <cstdio>
#include <limits>

#include "snmp_ber_tags.h"

namespace snmp {
namespace ber {

/**
 * @brief Helper для чтения INTEGER и подобных типов.
 *
 * @param p
 * @param end
 * @param expectedTag
 * @param outValue
 * @param err
 * @return true
 * @return false
 */
static bool readIntegerImpl(
    const uint8_t*& p, const uint8_t* end, uint8_t expectedTag, int& outValue, ErrorMessage& err);

// ------------------------------------------------------------
// readTagAndLength
// ------------------------------------------------------------
bool BerReader::readTagAndLength(
    const uint8_t*& p, const uint8_t* end, uint8_t expectedTag, size_t& outLen, ErrorMessage& err) {
    if (p >= end) {
        char buf[128];
        std::snprintf(
            buf, sizeof(buf), "Unexpected end of buffer while reading tag 0x%02X", expectedTag);
        err = buf;
        return false;
    }

    uint8_t tag = *p++;
    if (tag != expectedTag) {
        char buf[128];
        std::snprintf(
            buf, sizeof(buf), "Invalid tag: expected 0x%02X, got 0x%02X", expectedTag, tag);
        err = buf;
        return false;
    }

    if (p >= end) {
        err = "Unexpected end of buffer while reading ASN.1 length";
        return false;
    }

    uint8_t first = *p++;

    // проверка выхода за границы буфера
    auto checkBounds = [&](size_t len) -> bool {
        if (len > static_cast<size_t>(end - p)) {
            err = "ASN.1 length exceeds buffer";
            return false;
        }
        return true;
    };

    // Short form (длина в одном байте)
    if (first < 0x80) {
        outLen = first;
        return checkBounds(outLen);
    }

    // Long form (младшие 7 бит - количество байт длины)
    uint8_t count = first & 0x7F;
    if (count == 0) {
        err = "Invalid ASN.1 length (indefinite form not supported)";
        return false;
    }

    if (count > sizeof(size_t)) {
        err = "ASN.1 length too large";
        return false;
    }

    if (count > static_cast<size_t>(end - p)) {
        err = "ASN.1 length field exceeds buffer";
        return false;
    }

    // собираем длину из следующих count байт
    size_t len = 0;
    for (uint8_t i = 0; i < count; ++i) {
        len = (len << 8) | (*p++);
    }

    outLen = len;
    return checkBounds(outLen);
}

// ------------------------------------------------------------
// readSequence
// ------------------------------------------------------------
bool BerReader::readSequence(const uint8_t*& p,
                             const uint8_t* end,
                             const uint8_t*& seqEnd,
                             ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_SEQUENCE, len, err)) return false;

    seqEnd = p + len;
    return true;
}

// ------------------------------------------------------------
// readInteger
// ------------------------------------------------------------
bool BerReader::readInteger(const uint8_t*& p,
                            const uint8_t* end,
                            int& outValue,
                            ErrorMessage& err) {
    return readIntegerImpl(p, end, TAG_INTEGER, outValue, err);
}

bool BerReader::readIntegerWithTag(const uint8_t*& p,
                                   const uint8_t* end,
                                   uint8_t expectedTag,
                                   int& outValue,
                                   ErrorMessage& err) {
    return readIntegerImpl(p, end, expectedTag, outValue, err);
}

// ------------------------------------------------------------
// readOctetString
// ------------------------------------------------------------
bool BerReader::readOctetString(const uint8_t*& p,
                                const uint8_t* end,
                                std::string& outStr,  // NOLINT(bugprone-easily-swappable-parameters)
                                ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_OCTETSTRING, len, err)) return false;

    outStr.assign(reinterpret_cast<const char*>(p), len);
    p += len;
    return true;
}

// ------------------------------------------------------------
// readOid
// ------------------------------------------------------------
bool BerReader::readOid(const uint8_t*& p,
                        const uint8_t* end,
                        Oid& outOid,  // NOLINT(bugprone-easily-swappable-parameters)
                        ErrorMessage& err) {
    size_t len = 0;
    if (!readTagAndLength(p, end, TAG_OID, len, err)) return false;

    if (len == 0) {
        err = "ASN.1 OID length is zero";
        return false;
    }

    const uint8_t* oidEnd = p + len;

    uint8_t fb = *p++;
    int first = fb / 40;
    int second = fb % 40;

    outOid = std::to_string(first) + "." + std::to_string(second);

    int arc = 0;
    while (p < oidEnd) {
        uint8_t b = *p++;
        arc = (arc << 7) | (b & 0x7F);

        if ((b & 0x80) == 0) {
            outOid += "." + std::to_string(arc);
            arc = 0;
        }
    }

    if (arc != 0) {
        err = "ASN.1 OID ended with unfinished arc";
        return false;
    }

    return true;
}

// ------------------------------------------------------------
// skipValue
// ------------------------------------------------------------
bool BerReader::skipValue(const uint8_t*& p, const uint8_t* end, ErrorMessage& err) {
    if (p >= end) {
        err = "Unexpected end of buffer while skipping value";
        return false;
    }

    uint8_t tag = *p++;

    if (p >= end) {
        err = "Unexpected end of buffer while skipping value length";
        return false;
    }

    uint8_t first = *p++;
    size_t len = 0;

    if (first < 0x80) {
        len = first;
    } else {
        uint8_t count = first & 0x7F;
        if (count > sizeof(size_t)) {
            err = "ASN.1 skipping value length too large";
            return false;
        }
        if (count == 0 || count > static_cast<size_t>(end - p)) {
            err = "Invalid ASN.1 length while skipping value";
            return false;
        }
        for (uint8_t i = 0; i < count; ++i) {
            len = (len << 8) | (*p++);
        }
    }

    if (len > static_cast<size_t>(end - p)) {
        err = "ASN.1 skipped value exceeds buffer";
        return false;
    }

    p += len;
    (void)tag;
    return true;
}

static bool readIntegerImpl(
    const uint8_t*& p, const uint8_t* end, uint8_t expectedTag, int& outValue, ErrorMessage& err) {
    size_t len = 0;
    if (!BerReader::readTagAndLength(p, end, expectedTag, len, err)) return false;

    if (len == 0) {
        err = "ASN.1 INTEGER length is zero";
        return false;
    }

    if (len > sizeof(int)) {
        err = "ASN.1 INTEGER length too large";
        return false;
    }

    bool negative = (p[0] & 0x80) != 0;

    using UInt = unsigned int;

    UInt value = negative ? ~UInt(0) : UInt(0);

    for (size_t i = 0; i < len; ++i) {
        value = (value << 8) | UInt(p[i]);
    }

    p += len;

    const UInt minNegativeMagnitude = static_cast<UInt>(std::numeric_limits<int>::max()) + UInt(1);
    const int minIntValue = std::numeric_limits<int>::min();

    if (negative) {
        const UInt magnitude = (~value) + UInt(1);  // модуль отрицательного числа
        if (magnitude == minNegativeMagnitude) {
            outValue = minIntValue;
        } else {
            outValue = -static_cast<int>(magnitude);
        }
    } else {
        outValue = static_cast<int>(value);
    }
    return true;
}

}  // namespace ber
}  // namespace snmp
