#include <gtest/gtest.h>

#include "snmp_ber_utils.h"
#include "snmp_ber_writer.h"

using namespace snmp::ber;

class SnmpEncoderTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    BerWriter* w = nullptr;

    void SetUp() override {
        buf.clear();
        w = new BerWriter(buf);
    }

    void TearDown() override { delete w; }

    // Проверка коротких последовательностей
    void expectBytes(std::initializer_list<uint8_t> expected) {
        std::vector<uint8_t> exp(expected);
        EXPECT_EQ(buf, exp);
    }
};

// Тесты взяты из проекта ИБП Эмулятор
#if 1  // Часть 1 — OBJECT IDENTIFIER (OID)
// ============================================================
// Часть 1 — OBJECT IDENTIFIER (OID)
// ============================================================

// Тест 1.1: OID базовый
TEST_F(SnmpEncoderTest, EncodeOid_Simple) {
    snmp::ber::encodeOid(*w, "1.3.6.1.4.1.9999.1");
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

// Тест 1.2: OID со значениями > 127
TEST_F(SnmpEncoderTest, EncodeOid_Long) {
    snmp::ber::encodeOid(*w, "1.3.6.1.4.1.5000000.1");
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

// Тест 1.3: Все компоненты OID < 128 (простейший случай)
TEST_F(SnmpEncoderTest, EncodeOid_AllSmall) {
    snmp::ber::encodeOid(*w, "1.3.6.1.2.3.4.5.6");
    // clang-format off
    expectBytes({
        0x06, 0x08,  // TAG + length
        0x2B,        // first=1*40+3
        0x06, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06
    });
    // clang-format on
}

// Тест 1.4 Пустой OID
TEST_F(SnmpEncoderTest, EncodeOid_EmptyString) {
    snmp::ber::encodeOid(*w, "");
    expectBytes({ 0x06, 0x00 });
}

// Тест 1.5 В OID только один компонент "1"
TEST_F(SnmpEncoderTest, EncodeOid_SingleComponent) {
    snmp::ber::encodeOid(*w, "1");
    expectBytes({ 0x06, 0x00 });
}
#endif
