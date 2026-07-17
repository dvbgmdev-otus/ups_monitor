#include <gtest/gtest.h>

#include <vector>

#include "snmp_ber_reader.h"

using namespace snmp::ber;

/**
 * @brief Тесты BerReader::readTagAndLength
 *
 * Проверяются ошибки чтения tag и length,
 * включая обрывы буфера и неподдерживаемые формы ASN.1 length.
 */
class BerReaderReadTagAndLengthTest : public ::testing::Test {
protected:
    snmp::ErrorMessage err;
    size_t len{0};
};

#if 1 // Часть 1 — Обрывы буфера при чтении tag
// ============================================================
// Часть 1 — Обрывы буфера при чтении tag
// ============================================================

// Тест 1.1: Пустой буфер — невозможно прочитать tag
TEST_F(BerReaderReadTagAndLengthTest, ReadTagAndLength_EmptyBuffer) {
    std::vector<uint8_t> buf;
    const uint8_t* p = buf.data();
    const uint8_t* end = p;
    EXPECT_FALSE(BerReader::readTagAndLength(p, end, 0x02, len, err));
    EXPECT_EQ(err, "Unexpected end of buffer while reading tag 0x02");
}
#endif

#if 1 // Часть 2 — Некорректные формы ASN.1 length
// ============================================================
// Часть 2 — Некорректные формы ASN.1 length
// ============================================================

// Тест 2.1: Indefinite length (0x80) не поддерживается
TEST_F(BerReaderReadTagAndLengthTest, ReadTagAndLength_IndefiniteLength) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER
        0x80  // indefinite length
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::readTagAndLength(p, end, 0x02, len, err));
    EXPECT_EQ(err, "Invalid ASN.1 length (indefinite form not supported)");
}

// Тест 2.2: Long-form length не помещается в size_t
TEST_F(BerReaderReadTagAndLengthTest, ReadTagAndLength_LengthTooLarge) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02,                                      // INTEGER
        static_cast<uint8_t>(0x80 | (sizeof(size_t) + 1))
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::readTagAndLength(p, end, 0x02, len, err));
    EXPECT_EQ(err, "ASN.1 length too large");
}
#endif

#if 1 // Часть 3 — Обрыв поля длины (long form)
// ============================================================
// Часть 3 — Обрыв поля длины (long form)
// ============================================================

// Тест 3.1: Long-form length field выходит за buffer
TEST_F(BerReaderReadTagAndLengthTest, ReadTagAndLength_TruncatedLengthField) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER
        0x82, // long-form, 2 bytes length expected
        0x01  // но есть только 1 байт
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::readTagAndLength(p, end, 0x02, len, err));
    EXPECT_EQ(err, "ASN.1 length field exceeds buffer");
}
#endif

#if 1 // Часть 4 — Значение выходит за границы буфера
// ============================================================
// Часть 4 — Значение выходит за границы буфера
// ============================================================

// Тест 4.1: Short-form length больше оставшегося buffer
TEST_F(BerReaderReadTagAndLengthTest, ReadTagAndLength_ShortFormValueExceedsBuffer) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER
        0x02, // short-form, value length = 2
        0x01  // но есть только 1 байт value
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::readTagAndLength(p, end, 0x02, len, err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// Тест 4.2: Long-form length больше оставшегося buffer
TEST_F(BerReaderReadTagAndLengthTest, ReadTagAndLength_LongFormValueExceedsBuffer) {
    // clang-format off
    std::vector<uint8_t> buf = {
        0x02, // INTEGER
        0x81, // long-form, 1 byte length
        0x02, // value length = 2
        0x01  // но есть только 1 байт value
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = p + buf.size();
    EXPECT_FALSE(BerReader::readTagAndLength(p, end, 0x02, len, err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}
#endif
