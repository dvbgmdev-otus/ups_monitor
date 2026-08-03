/**
 * @file snmp_ber_tags.h
 * @ingroup snmp_ber
 * @brief ASN.1 BER теги, используемые SNMP-кодеком.
 */
#ifndef SNMP_BER_TAGS_H
#define SNMP_BER_TAGS_H

#include <cstdint>

namespace snmp {
namespace ber {

// =========================================================
// Universal class (00)
// =========================================================
enum : uint8_t {
    TAG_INTEGER     = 0x02,  ///< Universal INTEGER.
    TAG_OCTETSTRING = 0x04,  ///< Universal OCTET STRING.
    TAG_NULL        = 0x05,  ///< Universal NULL.
    TAG_OID         = 0x06,  ///< Universal OBJECT IDENTIFIER.
    TAG_SEQUENCE    = 0x30   ///< Universal constructed SEQUENCE.
};

// =========================================================
// Application class (01)
// SNMP-specific scalar types
// =========================================================
enum : uint8_t {
    TAG_COUNTER32 = 0x41,  ///< SNMP Counter32.
    TAG_GAUGE32   = 0x42,  ///< SNMP Gauge32.
    TAG_TIMETICKS = 0x43,  ///< SNMP TimeTicks.
    TAG_COUNTER64 = 0x46   ///< SNMP Counter64.
};

// =========================================================
// Context-specific class (10)
// SNMP PDUs
// =========================================================
enum : uint8_t {
    TAG_GETREQUEST  = 0xA0,  ///< SNMP GetRequest-PDU.
    TAG_GETRESPONSE = 0xA2   ///< SNMP GetResponse-PDU.
};

}  // namespace ber
}  // namespace snmp

#endif  // SNMP_BER_TAGS_H
