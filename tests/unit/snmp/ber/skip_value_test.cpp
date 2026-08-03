#include <gtest/gtest.h>

#include <vector>

#include "snmp_ber_reader.h"

using namespace snmp::ber;

/**
 * @brief Тесты BerReader::skipValue
 *
 * Проверяется корректное пропускание ASN.1 TLV
 * и обработка ошибок при некорректных буферах.
 *
 * skipValue НЕ интерпретирует tag и value,
 * а только безопасно перемещает курсор.
 */
class BerReaderSkipValueTest : public ::testing::Test {
protected:
    snmp::ErrorMessage err;
};

#if 1  // Часть 1 — Корректные TLV
// ============================================================
// Часть 1 — Корректные TLV
// ============================================================

// Тест 1.1: Short length (1 byte value)
TEST_F(BerReaderSkipValueTest, SkipValue_ShortLength) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER tag
        0x01, // length
        0x7F  // value
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_TRUE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(p, end);
    EXPECT_TRUE(err.empty());
}

// Тест 1.2: Long-form length (2 bytes length field)
TEST_F(BerReaderSkipValueTest, SkipValue_LongLength) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x04,       // OCTET STRING
        0x82, 0x00, 0x03, // length = 3
        0xAA, 0xBB, 0xCC
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_TRUE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(p, end);
    EXPECT_TRUE(err.empty());
}
#endif

#if 1  // Часть 2 — Обрывы буфера
// ============================================================
// Часть 2 — Обрывы буфера
// ============================================================

// Тест 2.1: Пустой буфер
TEST_F(BerReaderSkipValueTest, SkipValue_EmptyBuffer) {
    std::vector<uint8_t> buf;
    const uint8_t* p = buf.data();
    const uint8_t* end = p;
    EXPECT_FALSE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(err, "Unexpected end of buffer while skipping value");
}

// Тест 2.2: Есть tag, но нет length
TEST_F(BerReaderSkipValueTest, SkipValue_MissingLength) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02 // INTEGER tag
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(err, "Unexpected end of buffer while skipping value length");
}
#endif

#if 1  // Часть 3 — Некорректный ASN.1 length
// ============================================================
// Часть 3 — Некорректный ASN.1 length
// ============================================================

// Тест 3.1: Indefinite length (0x80) — запрещено
TEST_F(BerReaderSkipValueTest, SkipValue_IndefiniteLength) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER
        0x80  // indefinite length
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(err, "Invalid ASN.1 length while skipping value");
}

// Тест 3.2: Long length, но length field обрывается
TEST_F(BerReaderSkipValueTest, SkipValue_TruncatedLengthField) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02,
        0x82, 0x01 // заявлено 2 байта длины, но есть только 1
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(err, "Invalid ASN.1 length while skipping value");
}

// Тест 3.3: Long-form length не помещается в size_t
TEST_F(BerReaderSkipValueTest, SkipValue_LengthTooLarge) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02,                                      // INTEGER
        static_cast<uint8_t>(0x80 | (sizeof(size_t) + 1))
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(err, "ASN.1 skipping value length too large");
}
#endif

#if 1  // Часть 4 — Value выходит за границы буфера
// ============================================================
// Часть 4 — Value выходит за границы буфера
// ============================================================

// Тест 4.1: Length больше доступного value
TEST_F(BerReaderSkipValueTest, SkipValue_ValueExceedsBuffer) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER
        0x02, // length = 2
        0x01  // value truncated
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::skipValue(p, end, err));
    EXPECT_EQ(err, "ASN.1 skipped value exceeds buffer");
}
#endif
