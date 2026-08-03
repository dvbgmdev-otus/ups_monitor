#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "snmp_ber_utils.h"
#include "snmp_ber_writer.h"

using namespace snmp::ber;

/**
 * @brief Тесты кодирования ASN.1 BER примитивов.
 *
 * Проверяются низкоуровневые encoder-функции:
 *  - INTEGER
 *  - OCTET STRING
 *  - NULL
 *  - OBJECT IDENTIFIER
 *
 * Тесты не зависят от SNMP codec или бизнес-логики.
 */
class SnmpBerEncodeTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    BerWriter* w{ nullptr };

    void SetUp() override {
        buf.clear();
        w = new BerWriter(buf);
    }

    void TearDown() override {
        delete w;
        w = nullptr;
    }

    void expectBytes(std::initializer_list<uint8_t> expected) {
        EXPECT_EQ(buf, std::vector<uint8_t>(expected));
    }
};

#if 1 // Часть 1 — ASN.1 INTEGER
// ============================================================
// Часть 1 — ASN.1 INTEGER
// ============================================================

// Тест 1.1: INTEGER = 0
TEST_F(SnmpBerEncodeTest, EncodeInteger_Zero) {
    encodeInteger(*w, 0);
    expectBytes({ 0x02, 0x01, 0x00 });
}

// Тест 1.2: INTEGER > 0
TEST_F(SnmpBerEncodeTest, EncodeInteger_Positive) {
    encodeInteger(*w, 5);
    expectBytes({ 0x02, 0x01, 0x05 });
}

// Тест 1.3: INTEGER < 0
TEST_F(SnmpBerEncodeTest, EncodeInteger_Negative) {
    encodeInteger(*w, -5);
    expectBytes({ 0x02, 0x01, 0xFB });
}

// Тест 1.4: INTEGER > 127 должно занимать 2 байта
TEST_F(SnmpBerEncodeTest, EncodeInteger_BigPositive) {
    // 128 = 0x00 0x80 (чтобы не было знакового бита)
    encodeInteger(*w, 128);
    // clang-format off
    expectBytes({
        0x02, 0x02, // TAG + length
        0x00, 0x80
    });
    // clang-format on
}

// Тест 1.5: INTEGER большое положительное (4 байта)
TEST_F(SnmpBerEncodeTest, EncodeInteger_LargePositive4Bytes) {
    encodeInteger(*w, 404719978);  // 0x18 1F 89 6A
    // clang-format off
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x18, 0x1F, 0x89, 0x6A  // value
    });
    // clang-format on
}

// Тест 1.6: INTEGER = -128 (минимальное однобайтовое значение)
TEST_F(SnmpBerEncodeTest, EncodeInteger_Negative128) {
    encodeInteger(*w, -128);
    // clang-format off
    expectBytes({
        0x02, 0x01,  // INTEGER, length 1
        0x80         // -128
    });
    // clang-format on
}

// Тест 1.7: INTEGER = -129 (требует 2 байта)
TEST_F(SnmpBerEncodeTest, EncodeInteger_Negative129) {
    encodeInteger(*w, -129);
    // clang-format off
    expectBytes({
        0x02, 0x02,  // INTEGER, length 2
        0xFF, 0x7F   // -129
    });
    // clang-format on
}

// Тест 1.8: INTEGER = INT_MAX (2147483647)
TEST_F(SnmpBerEncodeTest, EncodeInteger_MaxInt32) {
    encodeInteger(*w, 2147483647);
    // clang-format off
    expectBytes({
        0x02, 0x04,             // INTEGER, length 4
        0x7F, 0xFF, 0xFF, 0xFF  // value
    });
    // clang-format on
}

// Тест 1.9: INTEGER = INT_MIN (-2147483648)
TEST_F(SnmpBerEncodeTest, EncodeInteger_MinInt32) {
    encodeInteger(*w, -2147483648LL);
    // clang-format off
    expectBytes({
        0x02, 0x04,            // INTEGER, length 4
        0x80, 0x00, 0x00, 0x00 // value
    });
    // clang-format on
}
#endif

#if 1 // Часть 2 — OCTET STRING
// ============================================================
// Часть 2 — OCTET STRING
// ============================================================

// Тест 2.1: Пустая строка
TEST_F(SnmpBerEncodeTest, EncodeOctetString_Empty) {
    encodeOctetString(*w, "");
    expectBytes({ 0x04, 0x00 });
}

// Тест 2.2: Текстовая строка - один символ
TEST_F(SnmpBerEncodeTest, EncodeOctetString_SingleChar) {
    encodeOctetString(*w, "A");
    expectBytes({ 0x04, 0x01, 0x41 });
}

// Тест 2.3:Текстовая строка "Hello"
TEST_F(SnmpBerEncodeTest, EncodeOctetString_Text) {
    encodeOctetString(*w, "Hello");
    expectBytes({ 0x04, 0x05, 'H', 'e', 'l', 'l', 'o' });
}
#endif

#if 1 // Часть 3 — ASN.1 NULL
// ============================================================
// Часть 3 — ASN.1 NULL
// ============================================================

TEST_F(SnmpBerEncodeTest, EncodeNull) {
    encodeNull(*w);
    expectBytes({ 0x05, 0x00 }); // TAG_NULL, length=0
}
#endif

#if 1 // Часть 4 — OBJECT IDENTIFIER (OID)
// ============================================================
// Часть 4 — OBJECT IDENTIFIER (OID)
// ============================================================

// Тест 4.1: OID базовый
TEST_F(SnmpBerEncodeTest, EncodeOid_Simple) {
    encodeOid(*w, "1.3.6.1.4.1.9999.1");
    // clang-format off
    expectBytes({
        0x06, 0x08,              // TAG_OID, length=8
        0x2B,                    // 1*40 + 3
        0x06, 0x01, 0x04, 0x01,  // path
        0xCE, 0x0F,              // 9999 - CE 0F
        0x01                     // last component
    });
    // clang-format on
}

// Тест 4.2: OID со значениями > 127
TEST_F(SnmpBerEncodeTest, EncodeOid_Long) {
    encodeOid(*w, "1.3.6.1.4.1.5000000.1");
    // clang-format off
    expectBytes({
        0x06, 0x0A,              // TAG_OID, length=10
        0x2B,                    // 1*40 + 3
        0x06, 0x01, 0x04, 0x01,  // 6.1.4.1
        0x82, 0xB1, 0x96, 0x40,  // 5,000,000 encoded base-128
        0x01                     // 1
    });
    // clang-format on
}

// Тест 4.3: Все компоненты OID < 128 (простейший случай)
TEST_F(SnmpBerEncodeTest, EncodeOid_AllSmall) {
    encodeOid(*w, "1.3.6.1.2.3.4.5.6");
    // clang-format off
    expectBytes({
        0x06, 0x08,  // TAG + length
        0x2B,        // first=1*40+3
        0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
    });
    // clang-format on
}

// Тест 4.4 Пустой OID
TEST_F(SnmpBerEncodeTest, EncodeOid_EmptyString) {
    encodeOid(*w, "");
    expectBytes({ 0x06, 0x00 });
}

// Тест 4.5 В OID только один компонент "1"
TEST_F(SnmpBerEncodeTest, EncodeOid_SingleComponent) {
    encodeOid(*w, "1");
    expectBytes({ 0x06, 0x00 });
}
#endif
