#include <gtest/gtest.h>

#include "snmp_ber_reader.h"

using namespace snmp::ber;

class BerReaderOctetStringTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    snmp::ErrorMessage err;
};

// ============================================================
// Part 1 — Correct OCTET STRING decoding
// ============================================================

// 1.1 Пустая строка
TEST_F(BerReaderOctetStringTest, ReadOctetString_Empty) {
    buf = { 0x04, 0x00 };  // OCTET STRING, length = 0
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    std::string value;
    ASSERT_TRUE(BerReader::readOctetString(p, end, value, err)) << err;
    EXPECT_TRUE(value.empty());
}

// 1.2 Строка из одного символа
TEST_F(BerReaderOctetStringTest, ReadOctetString_SingleChar) {
    buf = { 0x04, 0x01, 'A' };
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    std::string value;
    ASSERT_TRUE(BerReader::readOctetString(p, end, value, err)) << err;
    EXPECT_EQ(value, "A");
}

// 1.3 Обычная ASCII-строка
TEST_F(BerReaderOctetStringTest, ReadOctetString_Text) {
    buf = { 0x04, 0x05, 'H', 'e', 'l', 'l', 'o' };
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    std::string value;
    ASSERT_TRUE(BerReader::readOctetString(p, end, value, err)) << err;
    EXPECT_EQ(value, "Hello");
}

// ============================================================
// Part 2 — Error cases
// ============================================================

// 2.1 Неверный тег (должен быть 0x04)
TEST_F(BerReaderOctetStringTest, ReadOctetString_InvalidTag_Error) {
    buf = { 0x02, 0x01, 0x00 };  // INTEGER вместо OCTET STRING
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    std::string value;
    ASSERT_FALSE(BerReader::readOctetString(p, end, value, err));
    EXPECT_EQ(err, "Invalid tag: expected 0x04, got 0x02");
}

// 2.2 Длина превышает буфер
TEST_F(BerReaderOctetStringTest, ReadOctetString_LengthExceedsBuffer_Error) {
    // clang-format off
    buf = {
        0x04, 0x05,  // length = 5
        'H', 'i'     // реально только 2 байта
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    std::string value;
    ASSERT_FALSE(BerReader::readOctetString(p, end, value, err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// 2.3 Буфер обрывается до length
TEST_F(BerReaderOctetStringTest, ReadOctetString_UnexpectedEnd_Error) {
    buf = { 0x04 };  // тег есть, length отсутствует
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    std::string value;
    ASSERT_FALSE(BerReader::readOctetString(p, end, value, err));
    EXPECT_EQ(err, "Unexpected end of buffer while reading ASN.1 length");
}
