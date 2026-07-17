#include <gtest/gtest.h>

#include "snmp_codec.h"

// ============================================================
// SNMP GET-RESPONSE decode error tests
// ============================================================

class SnmpCodecDecodeGetResponseErrorsTest : public ::testing::Test {
protected:
    snmp::codec::SnmpCodec codec;
    std::vector<snmp::codec::SnmpValue> values;
    snmp::ErrorMessage err;

    bool decode(const uint8_t* data, size_t size, int expectedRequestId = 0) {
        values.clear();
        err.clear();
        return codec.decodeGetResponse(data, size, expectedRequestId, values, err);
    }
};

#if 1  // Часть 1 — Ошибки верхнего уровня Message
// ============================================================
// Часть 1 — Ошибки верхнего уровня Message
// ============================================================

// Тест 1.1: Неверный тег верхнего SEQUENCE
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, InvalidTopLevelTag) {
    // Первый байт должен быть SEQUENCE (0x30)
    uint8_t data[] = {
        0x31, 0x00  // WRONG tag
    };
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// Тест 1.2: Длина SEQUENCE выходит за пределы буфера
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, MessageLengthExceedsBuffer) {
    // clang-format off
    uint8_t data[] = {
        0x30, 0x10, // SEQUENCE len = 16
        0x02, 0x01, 0x00
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

#endif

#if 1  // Часть 2 — Ошибки версии и community
// ============================================================
// Часть 2 — Ошибки версии и community
// ============================================================

// Тест 2.1: Неверный тег версии (должен быть INTEGER)
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, InvalidVersionTag) {
    // version tag = NULL (0x05) вместо INTEGER (0x02)
    // clang-format off
    uint8_t data[] = {
        0x30, 0x08,           // Message SEQUENCE
            0x05, 0x01, 0x00, // WRONG version tag
            0x04, 0x01, 'x',  // community
            0xA2, 0x00        // empty GetResponse-PDU
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Тест 2.2: Неподдерживаемая версия SNMP
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, UnsupportedSnmpVersion) {
    // SNMP version = 2 (unsupported)
    // clang-format off
    uint8_t data[] = {
        0x30, 0x13,             // Message SEQUENCE, len = 19
            0x02, 0x01, 0x02,   // version = 2 (unsupported)
            0x04, 0x01, 'x',    // community
            0xA2, 0x0B,         // GetResponse-PDU, len = 11
                0x02, 0x01, 0x01, // request-id
                0x02, 0x01, 0x00, // error-status
                0x02, 0x01, 0x00, // error-index
                0x30, 0x00        // VarBindList (EMPTY, но ОБЯЗАТЕЛЬНЫЙ)
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "Unsupported SNMP version (expected v1 or v2c)");
}

#endif

#if 1  // Часть 3 — Ошибки GetResponse-PDU
// =====================================================
// Часть 3 — Ошибки GetResponse-PDU
// ============================================================

// Тест 3.1: Неверный тег PDU (должен быть A2)
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, InvalidPduTag) {
    // clang-format off
    uint8_t data[] = {
        0x30, 0x08,
            0x02, 0x01, 0x00,
            0x04, 0x01, 'x',
            0xA0, 0x00 // GetRequest вместо GetResponse
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "Invalid tag: expected 0xA2, got 0xA0");
}

// Тест 3.2: request-id ответа не совпадает с ожидаемым request-id
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, RequestIdMismatch) {
    // clang-format off
    uint8_t data[] = {
        0x30, 0x13,
            0x02, 0x01, 0x00,
            0x04, 0x01, 'x',
            0xA2, 0x0B,
                0x02, 0x01, 0x01, // request-id = 1
                0x02, 0x01, 0x00,
                0x02, 0x01, 0x00,
                0x30, 0x00
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data), 2));
    EXPECT_EQ(err, "SNMP response request-id mismatch");
}

#endif

#if 1  // Часть 4 — Ошибки VarBindList и VarBind
// =====================================================
// Часть 4 — Ошибки VarBindList и VarBind
// ============================================================

// Тест 4.1: VarBindList имеет неверный тег
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, InvalidVarBindListTag) {
    // VarBindList имеет неверный тег (должен быть SEQUENCE = 0x30)
    // clang-format off
    uint8_t data[] = {
        0x30, 0x13,                 // Message SEQUENCE, len = 19
            0x02, 0x01, 0x00,       // version
            0x04, 0x01, 'x',        // community
            0xA2, 0x0B,             // GetResponse-PDU, len = 11
                0x02, 0x01, 0x00,   // request-id
                0x02, 0x01, 0x00,   // error-status
                0x02, 0x01, 0x00,   // error-index
                0x31, 0x00          // WRONG VarBindList tag
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// Тест 4.2: VarBind без поля value
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, VarBindMissingValue) {
    // VarBind содержит OID, но не содержит value
    // clang-format off
    uint8_t data[] = {
        0x30, 0x19,                 // Message SEQUENCE, len = 25
            0x02, 0x01, 0x00,       // version
            0x04, 0x01, 'x',        // community
            0xA2, 0x11,             // GetResponse-PDU, len = 17
                0x02, 0x01, 0x00,   // request-id
                0x02, 0x01, 0x00,   // error-status
                0x02, 0x01, 0x00,   // error-index
                0x30, 0x06,         // VarBindList, len = 6
                    0x30, 0x04,     // VarBind, len = 4
                        0x06, 0x02, 0x2B, 0x06 // OID = 1.3.6, НЕТ value
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "VarBind missing value field");
}

// Тест 4.3: VarBind с неподдерживаемым типом значения
TEST_F(SnmpCodecDecodeGetResponseErrorsTest, UnsupportedVarBindValueType) {
    // VarBind с неподдерживаемым типом значения (BOOLEAN)
    // clang-format off
    uint8_t data[] = {
        0x30, 0x1B,                 // Message SEQUENCE, len = 27
            0x02, 0x01, 0x00,       // version
            0x04, 0x01, 'x',        // community
            0xA2, 0x13,             // GetResponse-PDU, len = 19
                0x02, 0x01, 0x00,   // request-id
                0x02, 0x01, 0x00,   // error-status
                0x02, 0x01, 0x00,   // error-index
                0x30, 0x08,         // VarBindList, len = 8
                    0x30, 0x06,     // VarBind, len = 6
                        0x06, 0x02, 0x2B, 0x06, // OID = 1.3.6
                        0x01, 0x00             // BOOLEAN (unsupported)
    };
    // clang-format on
    ASSERT_FALSE(decode(data, sizeof(data)));
    EXPECT_EQ(err, "Unsupported SNMP value type (tag = 0x1)");
}
#endif
