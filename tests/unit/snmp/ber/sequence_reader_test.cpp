#include <gtest/gtest.h>

#include "snmp_ber_reader.h"

using namespace snmp::ber;

class BerReaderSequenceTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    snmp::ErrorMessage err;
};

// ============================================================
// Part 1 — Correct SEQUENCE decoding
// ============================================================

// 1.1 Пустая SEQUENCE: 30 00
TEST_F(BerReaderSequenceTest, ReadSequence_Empty) {
    buf = { 0x30, 0x00 };
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* seqEnd = nullptr;
    ASSERT_TRUE(BerReader::readSequence(p, end, seqEnd, err));
    EXPECT_EQ(p, seqEnd);
}

// 1.2 SEQUENCE с контентом
// 30 03 01 02 03
TEST_F(BerReaderSequenceTest, ReadSequence_WithContent) {
    buf = { 0x30, 0x03, 0x01, 0x02, 0x03 };
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* seqEnd = nullptr;
    ASSERT_TRUE(BerReader::readSequence(p, end, seqEnd, err));
    EXPECT_EQ(seqEnd - p, 3);
}

// 1.3 Вложенная SEQUENCE
// 30 05
//    30 03
//       01 02 03
TEST_F(BerReaderSequenceTest, ReadSequence_Nested) {
    // clang-format off
    buf = {
        0x30, 0x05,
            0x30, 0x03,
                0x01, 0x02, 0x03
    };
    // clang-format on
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* outerEnd = nullptr;
    ASSERT_TRUE(BerReader::readSequence(p, end, outerEnd, err));
    EXPECT_EQ(outerEnd, buf.data() + buf.size());
}

// 1.4 Long-form length
// SEQUENCE длиной 130 байт
TEST_F(BerReaderSequenceTest, ReadSequence_LongLength) {
    buf.clear();
    buf.push_back(0x30);
    buf.push_back(0x81);
    buf.push_back(0x82); // length = 130
    buf.insert(buf.end(), 130, 0xAA);
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* seqEnd = nullptr;
    ASSERT_TRUE(BerReader::readSequence(p, end, seqEnd, err));
    EXPECT_EQ(seqEnd, buf.data() + buf.size());
}

// ============================================================
// Part 2 — Error cases
// ============================================================

// 2.1 Неверный тег (не SEQUENCE)
TEST_F(BerReaderSequenceTest, ReadSequence_InvalidTag_Error) {
    buf = { 0x31, 0x00 }; // SET вместо SEQUENCE
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* seqEnd = nullptr;
    ASSERT_FALSE(BerReader::readSequence(p, end, seqEnd, err));
    EXPECT_EQ(err, "Invalid tag: expected 0x30, got 0x31");
}

// 2.2 Длина выходит за пределы буфера
TEST_F(BerReaderSequenceTest, ReadSequence_LengthExceedsBuffer_Error) {
    buf = { 0x30, 0x05, 0x01, 0x02 };
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* seqEnd = nullptr;
    ASSERT_FALSE(BerReader::readSequence(p, end, seqEnd, err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// 2.3 Неожиданный конец при чтении длины
TEST_F(BerReaderSequenceTest, ReadSequence_UnexpectedEnd_Error) {
    buf = { 0x30 }; // нет length
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* seqEnd = nullptr;
    ASSERT_FALSE(BerReader::readSequence(p, end, seqEnd, err));
    EXPECT_EQ(err, "Unexpected end of buffer while reading ASN.1 length");
}
