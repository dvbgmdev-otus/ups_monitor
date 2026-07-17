#include <gtest/gtest.h>

#include <cstring>

#include "snmp_codec.h"

using namespace snmp::codec;

// ============================================================
// SNMP GET-REQUEST encode tests
// ============================================================

class SnmpCodecEncodeGetRequestTest : public ::testing::Test {
protected:
    std::vector<uint8_t> out;
    snmp::ErrorMessage err;

    void encode(const SnmpGetRequest& req) {
        out.clear();
        err.clear();
        SnmpCodec::encodeGetRequest(req, out);
    }
};

#if 1  // Часть 1 — Корректное кодирование
// =====================================================
// Часть 1 — Корректное кодирование
// ============================================================

// Тест 1.1: GET-request с одним OID
TEST_F(SnmpCodecEncodeGetRequestTest, EncodeSingleOid) {
    SnmpGetRequest req;
    req.community = "public";
    req.requestId = 1;
    req.oids = { "1.3.6" };
    encode(req);
    // clang-format off
    const uint8_t expected[] = {
        0x30, 0x20,                 // Message SEQUENCE
            0x02, 0x01, 0x01,       // version = v2c
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA0, 0x13,             // GetRequest-PDU
                0x02, 0x01, 0x01,   // request-id
                0x02, 0x01, 0x00,   // error-status
                0x02, 0x01, 0x00,   // error-index
                0x30, 0x08,         // VarBindList
                    0x30, 0x06,     // VarBind
                        0x06, 0x02, 0x2B, 0x06, // OID 1.3.6
                        0x05, 0x00              // NULL
    };
    // clang-format on
    ASSERT_EQ(out.size(), sizeof(expected));
    EXPECT_EQ(0, std::memcmp(out.data(), expected, sizeof(expected)));
}

// Тест 1.2: GET-request с двумя OID
TEST_F(SnmpCodecEncodeGetRequestTest, EncodeTwoOids) {
    SnmpGetRequest req;
    req.community = "public";
    req.requestId = 42;
    req.oids = { "1.3.6", "1.3.7" };
    encode(req);
    // Проверяем только структуру и ключевые байты,
    // без полного побайтового сравнения
    ASSERT_GT(out.size(), 0u);
    EXPECT_EQ(out[0], 0x30);  // Message SEQUENCE
    EXPECT_EQ(out[2], 0x02);  // INTEGER (version)
    EXPECT_EQ(out[5], 0x04);  // OCTET STRING (community)
}
#endif
