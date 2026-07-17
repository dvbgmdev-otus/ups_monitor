#include <gtest/gtest.h>

#include "snmp_ber_reader.h"
#include "snmp_ber_utils.h"
#include "snmp_ber_writer.h"

using namespace snmp::ber;

class BerReaderOidTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    snmp::ErrorMessage err;

    // Удобный helper: кодируем OID и сразу читаем его
    bool encodeAndRead(const std::string& oidStr, snmp::Oid& outOid) {
        buf.clear();
        err.clear();
        // encode
        BerWriter w(buf);
        snmp::ber::encodeOid(w, oidStr);
        // decode
        const uint8_t* p = buf.data();
        const uint8_t* end = buf.data() + buf.size();
        return BerReader::readOid(p, end, outOid, err);
    }
};

// ============================================================
// Part 1 — Correct OID decoding
// ============================================================

// 1.1 Простой OID
TEST_F(BerReaderOidTest, ReadOid_Simple) {
    snmp::Oid oid;
    ASSERT_TRUE(encodeAndRead("1.3.6.1.4.1.9999.1", oid)) << err;
    EXPECT_EQ(oid, "1.3.6.1.4.1.9999.1");
}

// 1.2 OID с длинной base-128 дугой
TEST_F(BerReaderOidTest, ReadOid_LongArc) {
    snmp::Oid oid;
    ASSERT_TRUE(encodeAndRead("1.3.6.1.4.1.5000000.1", oid)) << err;
    EXPECT_EQ(oid, "1.3.6.1.4.1.5000000.1");
}

// 1.3 Все компоненты < 128
TEST_F(BerReaderOidTest, ReadOid_AllSmall) {
    snmp::Oid oid;
    ASSERT_TRUE(encodeAndRead("1.3.6.1.2.3.4.5.6", oid)) << err;
    EXPECT_EQ(oid, "1.3.6.1.2.3.4.5.6");
}

// ============================================================
// Part 2 — Error cases
// ============================================================

// 2.1 OID с длиной 0 (06 00)
TEST_F(BerReaderOidTest, ReadOid_LengthZero_Error) {
    buf = { 0x06, 0x00 };
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();

    snmp::Oid oid;
    ASSERT_FALSE(BerReader::readOid(p, end, oid, err));
    EXPECT_EQ(err, "ASN.1 OID length is zero");
}

// 2.2 Незавершённая base-128 дуга
TEST_F(BerReaderOidTest, ReadOid_UnfinishedArc_Error) {
    // 1.3 + незаконченное продолжение
    // clang-format off
    buf = {
        0x06, 0x02,
        0x2B,  // 1*40 + 3
        0x86   // continuation bit set, но байт обрывается
    };
    // clang-format on

    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();

    snmp::Oid oid;
    ASSERT_FALSE(BerReader::readOid(p, end, oid, err));
    EXPECT_EQ(err, "ASN.1 OID ended with unfinished arc");
}

// 2.3 Длина OID выходит за пределы буфера
TEST_F(BerReaderOidTest, ReadOid_LengthExceedsBuffer_Error) {
    // clang-format off
    buf = {
        0x06, 0x05, // length = 5
        0x2B, 0x06  // реально только 2 байта
    };
    // clang-format on

    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();

    snmp::Oid oid;
    ASSERT_FALSE(BerReader::readOid(p, end, oid, err));
    EXPECT_EQ(err, "ASN.1 length exceeds buffer");
}
