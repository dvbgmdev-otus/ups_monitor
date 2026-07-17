#include <gtest/gtest.h>

#include "snmp_codec.h"

// ============================================================
// SNMP GET-RESPONSE decode tests
// ============================================================

class SnmpCodecDecodeGetResponseTest : public ::testing::Test {
protected:
    snmp::codec::SnmpCodec codec;
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    bool decode(const uint8_t* data, size_t size, int expectedRequestId = 1) {
        values.clear();
        err.clear();
        return codec.decodeGetResponse(data, size, expectedRequestId, values, err);
    }

    bool decodeSingleApplicationInteger(uint8_t valueTag, uint8_t value) {
        // clang-format off
        uint8_t data[] = {
            0x30, 0x21,                 // Message SEQUENCE, len = 33
                0x02, 0x01, 0x00,       // version = v1
                0x04, 0x06, 'p','u','b','l','i','c',
                0xA2, 0x14,             // GetResponse-PDU, len = 20
                    0x02, 0x01, 0x01,   // request-id = 1
                    0x02, 0x01, 0x00,   // error-status = 0
                    0x02, 0x01, 0x00,   // error-index = 0
                    0x30, 0x09,         // VarBindList, len = 9
                        0x30, 0x07,     // VarBind, len = 7
                            0x06, 0x02, 0x2B, 0x06, // OID 1.3.6
                            valueTag, 0x01, value
        };
        // clang-format on
        return decode(data, sizeof(data));
    }
};

#if 1  // Часть 1 — Один VarBind
// =====================================================
// Часть 1 — Один VarBind
// ============================================================

// Тест 1.1: Один INTEGER VarBind
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeSingleIntegerValue) {
    // OID: 1.3.6, value = 5
    // clang-format off
    uint8_t data[] = {
        0x30, 0x21,                 // Message SEQUENCE, len = 33
            0x02, 0x01, 0x00,       // version = v1
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA2, 0x14,             // GetResponse-PDU, len = 20
                0x02, 0x01, 0x01,   // request-id = 1
                0x02, 0x01, 0x00,   // error-status = 0
                0x02, 0x01, 0x00,   // error-index = 0
                0x30, 0x09,         // VarBindList, len = 9
                    0x30, 0x07,     // VarBind, len = 7
                        0x06, 0x02, 0x2B, 0x06, // OID 1.3.6
                        0x02, 0x01, 0x05        // INTEGER = 5
    };
    // clang-format on
    ASSERT_TRUE(decode(data, sizeof(data))) << err;
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::Integer);
    EXPECT_EQ(values[0].intValue, 5);
}

// Тест 1.2: Один OCTET STRING VarBind
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeSingleStringValue) {
    // OID: 1.3.6, value = "UPS"
    // clang-format off
    uint8_t data[] = {
        0x30, 0x23,
            0x02, 0x01, 0x00,
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA2, 0x16,
                0x02, 0x01, 0x01,
                0x02, 0x01, 0x00,
                0x02, 0x01, 0x00,
                0x30, 0x0B,
                    0x30, 0x09,
                        0x06, 0x02, 0x2B, 0x06, // OID 1.3.6
                        0x04, 0x03, 'U','P','S'
    };
    // clang-format on
    ASSERT_TRUE(decode(data, sizeof(data))) << err;
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::String);
    EXPECT_EQ(values[0].strValue, "UPS");
}

// Тест 1.3: Один NULL VarBind
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeSingleNullValue) {
    // OID: 1.3.6, value = NULL
    // clang-format off
    uint8_t data[] = {
        0x30, 0x20,
            0x02, 0x01, 0x00,
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA2, 0x13,
                0x02, 0x01, 0x01,
                0x02, 0x01, 0x00,
                0x02, 0x01, 0x00,
                0x30, 0x08,
                    0x30, 0x06,
                        0x06, 0x02, 0x2B, 0x06,
                        0x05, 0x00
    };
    // clang-format on
    ASSERT_TRUE(decode(data, sizeof(data))) << err;
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::Null);
}

// Тест 1.4: Один Gauge32 VarBind
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeSingleGauge32Value) {
    ASSERT_TRUE(decodeSingleApplicationInteger(0x42, 5)) << err;
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::Integer);
    EXPECT_EQ(values[0].intValue, 5);
}

// Тест 1.5: Один Counter32 VarBind
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeSingleCounter32Value) {
    ASSERT_TRUE(decodeSingleApplicationInteger(0x41, 7)) << err;
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::Integer);
    EXPECT_EQ(values[0].intValue, 7);
}

// Тест 1.6: Один TimeTicks VarBind
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeSingleTimeTicksValue) {
    ASSERT_TRUE(decodeSingleApplicationInteger(0x43, 9)) << err;
    ASSERT_EQ(values.size(), 1u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::Integer);
    EXPECT_EQ(values[0].intValue, 9);
}
#endif

#if 1 // Часть 2 — Несколько VarBind
// =====================================================
// Часть 2 — Несколько VarBind
// ============================================================

// Тест 2.1: Два VarBind (INTEGER + STRING)
TEST_F(SnmpCodecDecodeGetResponseTest, DecodeTwoValues) {
    // OID1 = 1.3.6 -> 5
    // OID2 = 1.3.7 -> "OK"
    // clang-format off
    uint8_t data[] = {
        0x30, 0x2B,
            0x02, 0x01, 0x00,
            0x04, 0x06, 'p','u','b','l','i','c',
            0xA2, 0x1E,
                0x02, 0x01, 0x01,
                0x02, 0x01, 0x00,
                0x02, 0x01, 0x00,
                0x30, 0x13,
                    0x30, 0x07,
                        0x06, 0x02, 0x2B, 0x06,
                        0x02, 0x01, 0x05,
                    0x30, 0x08,
                        0x06, 0x02, 0x2B, 0x07,
                        0x04, 0x02, 'O','K'
    };
    // clang-format on
    ASSERT_TRUE(decode(data, sizeof(data))) << err;
    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0].oid, "1.3.6");
    EXPECT_EQ(values[0].type, snmp::codec::SnmpValue::Type::Integer);
    EXPECT_EQ(values[0].intValue, 5);
    EXPECT_EQ(values[1].oid, "1.3.7");
    EXPECT_EQ(values[1].type, snmp::codec::SnmpValue::Type::String);
    EXPECT_EQ(values[1].strValue, "OK");
}
#endif

#if 1 // Часть 3 — Ошибки GetResponse
// =====================================================
// Часть 3 — Ошибки GetResponse
// ============================================================

// Тест 3.1: error-status != 0
TEST_F(SnmpCodecDecodeGetResponseTest, ErrorStatusNonZero) {
    // error-status = 2
    // clang-format off
    uint8_t data[] = {
        0x30, 0x13,
            0x02, 0x01, 0x00,
            0x04, 0x01, 'x',
            0xA2, 0x0B,
                0x02, 0x01, 0x01,
                0x02, 0x01, 0x02, // error-status = 2
                0x02, 0x01, 0x00,
                0x30, 0x00
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "SNMP error-status = 2, error-index = 0");
}
#endif
