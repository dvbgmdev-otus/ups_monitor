/**
 * @file snmp_codec.cpp
 * @ingroup snmp
 * @brief Реализация SNMP-кодека для GET-request и GET-response.
 */
#include "snmp_codec.h"

#include "snmp_ber_reader.h"
#include "snmp_ber_tags.h"
#include "snmp_ber_utils.h"
#include "snmp_ber_writer.h"

namespace snmp {
namespace codec {

using namespace ber;

void SnmpCodec::encodeGetRequest(const SnmpGetRequest& req, std::vector<uint8_t>& out) {
    out.clear();

    BerWriter w(out);

    // -----------------------------------------------------
    // SNMP Message ::= SEQUENCE
    // -----------------------------------------------------
    size_t msgStart = w.beginSequence(TAG_SEQUENCE);

    // version
    encodeInteger(w, static_cast<int>(req.version));

    // community
    encodeOctetString(w, req.community);

    // -----------------------------------------------------
    // PDU ::= GetRequest-PDU
    // -----------------------------------------------------
    size_t pduStart = w.beginSequence(TAG_GETREQUEST);

    // request-id
    encodeInteger(w, req.requestId);

    // error-status = 0
    encodeInteger(w, 0);

    // error-index = 0
    encodeInteger(w, 0);

    // -----------------------------------------------------
    // VarBindList ::= SEQUENCE OF VarBind
    // -----------------------------------------------------
    size_t vblStart = w.beginSequence(TAG_SEQUENCE);

    for (const auto& oid : req.oids) {
        encodeGetVarBind(w, oid);
    }

    w.endSequence(vblStart);  // VarBindList
    w.endSequence(pduStart);  // PDU
    w.endSequence(msgStart);  // Message
}

void SnmpCodec::encodeGetVarBind(BerWriter& w, const Oid& oid) {
    // VarBind ::= SEQUENCE { name OID, value NULL }
    size_t vbStart = w.beginSequence(TAG_SEQUENCE);

    encodeOid(w, oid);
    encodeNull(w);

    w.endSequence(vbStart);
}

bool SnmpCodec::decodeGetResponse(const uint8_t* data,
                                  size_t size,  // NOLINT(bugprone-easily-swappable-parameters)
                                  int expectedRequestId,
                                  SnmpVersion expectedVersion,
                                  std::vector<SnmpValue>& out,
                                  ErrorMessage& err) {
    const uint8_t* p = data;
    const uint8_t* end = data + size;
    const uint8_t* msgEnd = nullptr;

    out.clear();

    // 1. SNMP Message SEQUENCE
    if (!BerReader::readSequence(p, end, msgEnd, err)) {
        return false;
    }

    // 2. Version
    int version = 0;
    if (!BerReader::readInteger(p, msgEnd, version, err)) {
        return false;
    }
    switch (version) {
        case static_cast<int>(SnmpVersion::V_1):
        case static_cast<int>(SnmpVersion::V_2C):
            break;
        default:
            err = "Unsupported SNMP version (expected v1 or v2c)";
            return false;
    }
    if (version != static_cast<int>(expectedVersion)) {
        err = "SNMP response version mismatch";
        return false;
    }

    // 3. Community
    std::string community;
    if (!BerReader::readOctetString(p, msgEnd, community, err)) {
        return false;
    }

    // 4. GetResponse-PDU
    size_t pduLen = 0;
    if (!BerReader::readTagAndLength(p, msgEnd, TAG_GETRESPONSE, pduLen, err)) {
        return false;
    }

    const uint8_t* pduEnd = p + pduLen;

    // 5. request-id
    int requestId = 0;
    if (!BerReader::readInteger(p, pduEnd, requestId, err)) {
        return false;
    }
    if (requestId != expectedRequestId) {
        err = "SNMP response request-id mismatch";
        return false;
    }

    // 6. error-status / error-index
    int errorStatus = 0;
    if (!BerReader::readInteger(p, pduEnd, errorStatus, err)) {
        return false;
    }
    int errorIndex = 0;
    if (!BerReader::readInteger(p, pduEnd, errorIndex, err)) {
        return false;
    }
    if (errorStatus != 0) {
        err = "SNMP error-status = " + std::to_string(errorStatus) +
              ", error-index = " + std::to_string(errorIndex);
        return false;
    }

    // 7. VarBindList
    if (!decodeVarBindList(p, pduEnd, out, err)) {
        return false;
    }

    return true;
}

bool SnmpCodec::decodeVarBindList(const uint8_t*& p,
                                  const uint8_t* end,
                                  std::vector<SnmpValue>& out,
                                  ErrorMessage& err) {
    const uint8_t* vblEnd = nullptr;
    if (!BerReader::readSequence(p, end, vblEnd, err)) {
        return false;
    }

    while (p < vblEnd) {
        SnmpValue value;
        if (!decodeVarBind(p, vblEnd, value, err)) return false;
        out.push_back(value);
    }

    return true;
}

bool SnmpCodec::decodeVarBind(const uint8_t*& p,
                              const uint8_t* end,
                              SnmpValue& out,
                              ErrorMessage& err) {
    const uint8_t* vbEnd = nullptr;
    if (!BerReader::readSequence(p, end, vbEnd, err)) {
        return false;
    }

    // --- OID ---
    Oid oid;
    if (!BerReader::readOid(p, vbEnd, oid, err)) {
        return false;
    }
    out.oid = oid;

    // --- VALUE ---
    if (p >= vbEnd) {
        err = "VarBind missing value field";
        return false;
    }

    uint8_t tag = *p;

    switch (tag) {
        case TAG_INTEGER:
        case TAG_GAUGE32:
        case TAG_COUNTER32:
        case TAG_TIMETICKS: {
            int v = 0;
            if (!BerReader::readIntegerWithTag(p, vbEnd, tag, v, err)) {
                return false;
            }
            out.type = SnmpValue::Type::Integer;
            out.intValue = v;
            break;
        }

        case TAG_OCTETSTRING: {
            std::string s;
            if (!BerReader::readOctetString(p, vbEnd, s, err)) {
                return false;
            }
            out.type = SnmpValue::Type::String;
            out.strValue = s;
            break;
        }

        case TAG_NULL: {
            size_t len = 0;
            if (!BerReader::readTagAndLength(p, vbEnd, TAG_NULL, len, err)) {
                return false;
            }
            out.type = SnmpValue::Type::Null;
            break;
        }

        default:
            err = "Unsupported SNMP value type (tag = 0x" + std::to_string(tag) + ")";
            return false;
    }

    return true;
}

}  // namespace codec
}  // namespace snmp
