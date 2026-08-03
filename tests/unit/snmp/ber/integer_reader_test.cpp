/**
 * @file integer_reader_test.cpp
 * @brief Тесты чтения ASN.1 BER INTEGER.
 */
#include <gtest/gtest.h>

#include <limits>
#include <vector>

#include "snmp_ber_reader.h"
#include "snmp_ber_tags.h"

using namespace snmp::ber;

class BerReaderIntegerTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    snmp::ErrorMessage err;
    bool decode(int& out) {
        const uint8_t* p = buf.data();
        const uint8_t* end = buf.data() + buf.size();
        err.clear();
        return BerReader::readInteger(p, end, out, err);
    }
};

class BerReaderIntegerWithTagTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    snmp::ErrorMessage err;

    bool decodeWithTag(uint8_t expectedTag, int& out) {
        const uint8_t* p = buf.data();
        const uint8_t* end = buf.data() + buf.size();
        err.clear();
        return BerReader::readIntegerWithTag(p, end, expectedTag, out, err);
    }
};

#if (1)  // Часть 1 — Корректное декодирование INTEGER
// ============================================================
// Часть 1 — Корректное декодирование INTEGER
// ============================================================

// Тест 1.1: INTEGER = 0
TEST_F(BerReaderIntegerTest, ReadInteger_Zero) {
    buf = { 0x02, 0x01, 0x00 };
    int value = -1;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, 0);
}

// Тест 1.2: INTEGER = 5
TEST_F(BerReaderIntegerTest, ReadInteger_Positive) {
    buf = { 0x02, 0x01, 0x05 };
    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, 5);
}

// Тест 1.3: INTEGER = -1
TEST_F(BerReaderIntegerTest, ReadInteger_Negative) {
    buf = { 0x02, 0x01, 0xFF };
    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, -1);
}

// Тест 1.4: INTEGER = -128
TEST_F(BerReaderIntegerTest, ReadInteger_Negative128) {
    buf = { 0x02, 0x01, 0x80 };

    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, -128);
}

// Тест 1.5: INTEGER = -129
TEST_F(BerReaderIntegerTest, ReadInteger_Negative129) {
    buf = { 0x02, 0x02, 0xFF, 0x7F };

    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, -129);
}

// Тест 1.6: INTEGER = 128 занимает 2 байта
TEST_F(BerReaderIntegerTest, ReadInteger_128) {
    buf = { 0x02, 0x02, 0x00, 0x80 };

    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, 128);
}

// Тест 1.7: INTEGER = 404719978 (0x18 1F 89 6A)
TEST_F(BerReaderIntegerTest, ReadInteger_LargePositive) {
    // clang-format off
    buf = {
        0x02, 0x04,
        0x18, 0x1F, 0x89, 0x6A
    };
    //clang-format on
    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, 404719978);
}

// Тест 1.8: INTEGER = INT_MAX (2147483647)
TEST_F(BerReaderIntegerTest, ReadInteger_MaxInt32) {
    // clang-format off
    buf = {
        0x02, 0x04,
        0x7F, 0xFF, 0xFF, 0xFF
    };
    //clang-format on
    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, std::numeric_limits<int>::max());
}

// Тест 1.9: INTEGER = INT_MIN (-2147483648)
TEST_F(BerReaderIntegerTest, ReadInteger_MinInt32) {
    // clang-format off
    buf = {
        0x02, 0x04,
        0x80, 0x00, 0x00, 0x00
    };
    //clang-format on
    int value = 0;
    ASSERT_TRUE(decode(value)) << err;
    EXPECT_EQ(value, std::numeric_limits<int>::min());
}
#endif

#if (1)  // Часть 2 — Ошибки чтения INTEGER
// ============================================================
// Часть 2 — Ошибки чтения INTEGER
// ============================================================

// Тест 2.1: length = 0
TEST_F(BerReaderIntegerTest, ReadInteger_LengthZero_Error) {
    buf = { 0x02, 0x00 };
    int value = 0;
    ASSERT_FALSE(decode(value));
    EXPECT_EQ(err, "ASN.1 INTEGER length is zero");
}

// Тест 2.2: length больше оставшегося buffer
TEST_F(BerReaderIntegerTest, ReadInteger_LengthExceedsBuffer_Error) {
    // clang-format off
    buf = {
        0x02, 0x04,
        0x01, 0x02   // только 2 байта вместо 4
    };
    //clang-format on
    int value = 0;
    ASSERT_FALSE(decode(value));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}

// Тест 2.3: неверный tag
TEST_F(BerReaderIntegerTest, ReadInteger_InvalidTag_Error) {
    buf = {
        0x05, 0x01, 0x00   // NULL вместо INTEGER
    };
    int value = 0;
    ASSERT_FALSE(decode(value));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x05");
}

// Тест 2.4: length больше размера int
TEST_F(BerReaderIntegerTest, ReadInteger_LengthTooLarge_Error) {
    buf = { 0x02, static_cast<uint8_t>(sizeof(int) + 1) };
    buf.insert(buf.end(), sizeof(int) + 1, 0x00);

    int value = 0;
    ASSERT_FALSE(decode(value));
    EXPECT_EQ(err, "ASN.1 INTEGER length too large");
}
#endif

#if (1)  // Часть 3 — INTEGER-like типы с custom tag
// ============================================================
// Часть 3 — INTEGER-like типы с custom tag
// ============================================================

// Тест 3.1: Gauge32 = 230
TEST_F(BerReaderIntegerWithTagTest, ReadGauge32_Ok) {
    // TAG_GAUGE32 = 0x42
    buf = { TAG_GAUGE32, 0x02, 0x00, 0xE6 };
    int value = 0;
    ASSERT_TRUE(decodeWithTag(TAG_GAUGE32, value)) << err;
    EXPECT_EQ(value, 230);
}

// Тест 3.2: Counter32 = 100
TEST_F(BerReaderIntegerWithTagTest, ReadCounter32_Ok) {
    buf = { TAG_COUNTER32, 0x01, 0x64 };
    int value = 0;
    ASSERT_TRUE(decodeWithTag(TAG_COUNTER32, value)) << err;
    EXPECT_EQ(value, 100);
}

// Тест 3.3: TimeTicks = 500
TEST_F(BerReaderIntegerWithTagTest, ReadTimeTicks_Ok) {
    buf = { TAG_TIMETICKS, 0x02, 0x01, 0xF4 };
    int value = 0;
    ASSERT_TRUE(decodeWithTag(TAG_TIMETICKS, value)) << err;
    EXPECT_EQ(value, 500);
}

// Тест 3.4: expectedTag не совпадает с real tag
TEST_F(BerReaderIntegerWithTagTest, ReadIntegerWithTag_WrongExpectedTag_Error) {
    buf = { TAG_GAUGE32, 0x01, 0x01 };
    int value = 0;
    ASSERT_FALSE(decodeWithTag(TAG_INTEGER, value));
    EXPECT_EQ(err, "Invalid tag: expected 0x02, got 0x42");
}

// Тест 3.5: real tag не совпадает с expectedTag
TEST_F(BerReaderIntegerWithTagTest, ReadIntegerWithTag_WrongRealTag_Error) {
    buf = { TAG_OCTETSTRING, 0x01, 0x01 };
    int value = 0;
    ASSERT_FALSE(decodeWithTag(TAG_GAUGE32, value));
    EXPECT_EQ(err, "Invalid tag: expected 0x42, got 0x04");
}
#endif
