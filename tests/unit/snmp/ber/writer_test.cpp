#include <gtest/gtest.h>
#include <vector>

#include "snmp_ber_writer.h"

using namespace snmp::ber;

class BerWriterTest : public ::testing::Test {
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

// =======================================================
// Часть 1 — putTag / putByte / putBytes
// =======================================================

// Тест 1.1: putTag
TEST_F(BerWriterTest, PutTag_WritesSingleByte) {
    w->putTag(0x02);
    expectBytes({ 0x02 });
}

// Тест 1.2: putByte
TEST_F(BerWriterTest, PutByte_WritesByte) {
    w->putByte(0xFF);
    expectBytes({ 0xFF });
}

// Тест 1.3: putBytes
TEST_F(BerWriterTest, PutBytes_WritesMultipleBytes) {
    uint8_t raw[] = { 0x01, 0x02, 0x03 };
    w->putBytes(raw, 3);
    expectBytes({ 0x01, 0x02, 0x03 });
}

// =======================================================
// Часть 2 — тесты для putLength
// =======================================================

// Тест 2.1: putLength short
TEST_F(BerWriterTest, PutLength_ShortForm) {
    w->putLength(5);
    expectBytes({ 0x05 });
}

// Тест 2.2: putLength long form (1 byte length)
// Пример: length = 200 = 0xC8
// 0x81 0xC8
TEST_F(BerWriterTest, PutLength_LongForm_OneByte) {
    w->putLength(200);
    expectBytes({ 0x81, 0xC8 });
}

// Тест 2.3: putLength long form (2 bytes)
// Пример: 500 = 0x01F4
// 0x82 0x01 0xF4
TEST_F(BerWriterTest, PutLength_LongForm_TwoBytes) {
    w->putLength(500); // 0x01F4
    expectBytes({ 0x82, 0x01, 0xF4 });
}

// =======================================================
// Часть 3 — beginSequence / endSequence
// =======================================================

// Тест 3.1: пустая SEQUENCE
// 30 00
TEST_F(BerWriterTest, Sequence_Empty) {
    size_t anchor = w->beginSequence(0x30);
    w->endSequence(anchor);
    expectBytes({ 0x30, 0x00 });
}

// TEST 3.2: SEQUENCE с контентом
// 30 03
//    01 02 03
TEST_F(BerWriterTest, Sequence_WithContent) {
    size_t anchor = w->beginSequence(0x30);
    uint8_t raw[] = {0x01, 0x02, 0x03};
    w->putBytes(raw, 3);
    w->endSequence(anchor);
    expectBytes({ 0x30, 0x03, 0x01, 0x02, 0x03 });
}

// =======================================================
// Часть 4 — вложенные SEQUENCE
// =======================================================
// Типичная ASN.1 структура:
// 30 05
//    30 03
//       01 02 03
// Тест 4.1: nested sequence
TEST_F(BerWriterTest, Sequence_Nested) {
    size_t outer = w->beginSequence(0x30);
    size_t inner = w->beginSequence(0x30);
    uint8_t raw[] = {0x01, 0x02, 0x03};
    w->putBytes(raw, 3);
    w->endSequence(inner);
    w->endSequence(outer);
    // clang-format off
    expectBytes({0x30, 0x05,  // outer
                 0x30, 0x03,  // inner
                 0x01, 0x02, 0x03});
    // clang-format on
}

// =======================================================
// Часть 5 — длинная форма длины внутри SEQUENCE
// =======================================================
// Например, SEQUENCE с 300 байтами:
// 30 82 01 2C  <300 bytes>
// Тест 5.1: SEQUENCE с длиной > 127
TEST_F(BerWriterTest, Sequence_LongLength) {
    size_t anchor = w->beginSequence(0x30);
    std::vector<uint8_t> tmp(300, 0xAA);
    w->putBytes(tmp.data(), tmp.size());
    w->endSequence(anchor);
    // Начало: 0x30 0x82 0x01 0x2C
    ASSERT_GE(buf.size(), 4u);
    EXPECT_EQ(buf[0], 0x30);
    EXPECT_EQ(buf[1], 0x82);
    EXPECT_EQ(buf[2], 0x01);
    EXPECT_EQ(buf[3], 0x2C);
    // Итого должно быть 4 + 300 = 304 байт
    EXPECT_EQ(buf.size(), 304u);
}
