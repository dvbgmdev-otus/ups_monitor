#include <gtest/gtest.h>

#include "snmp_ber_reader.h"
#include "snmp_ber_utils.h"
#include "snmp_ber_writer.h"
#include "snmp_client.h"

using namespace snmp::ber;
using namespace snmp::codec;

class BerReaderVarBindTest : public ::testing::Test {
protected:
    std::vector<uint8_t> buf;
    snmp::ErrorMessage err;

    bool encodeAndReadSingle(const std::string& oidStr,
                             const std::function<void(BerWriter&)>& writeValue,
                             snmp::Oid& outOid,
                             SnmpValue& outVal) {
        buf.clear();
        err.clear();

        // --- encode VarBind ---
        BerWriter w(buf);
        size_t vbStart = w.beginSequence(0x30);  // SEQUENCE

        snmp::ber::encodeOid(w, oidStr);
        writeValue(w);

        w.endSequence(vbStart);

        // --- decode VarBind ---
        const uint8_t* p = buf.data();
        const uint8_t* end = buf.data() + buf.size();

        const uint8_t* vbEnd = nullptr;
        if (!BerReader::readSequence(p, end, vbEnd, err)) return false;
        if (!BerReader::readOid(p, vbEnd, outOid, err)) return false;

        uint8_t tag = *p;
        if (tag == 0x02) {
            int v = 0;
            if (!BerReader::readInteger(p, vbEnd, v, err)) return false;
            outVal.type = SnmpValue::Type::Integer;
            outVal.intValue = v;
        } else if (tag == 0x04) {
            std::string s;
            if (!BerReader::readOctetString(p, vbEnd, s, err)) return false;
            outVal.type = SnmpValue::Type::String;
            outVal.strValue = s;
        } else if (tag == 0x05) {
            size_t len = 0;
            if (!BerReader::readTagAndLength(p, vbEnd, 0x05, len, err)) return false;
            outVal.type = SnmpValue::Type::Null;
        } else {
            err = "Unsupported VarBind value tag";
            return false;
        }

        return true;
    }
};

// ============================================================
// Part 1 — Valid VarBind
// ============================================================

// 1.1 VarBind с INTEGER
TEST_F(BerReaderVarBindTest, VarBind_Integer) {
    snmp::Oid oid;
    SnmpValue val;
    ASSERT_TRUE(encodeAndReadSingle(
        "1.3.6.1.2.1.1.1.0",
        [](BerWriter& w) {
            w.putTag(0x02);
            w.putLength(1);
            w.putByte(42);
        },
        oid,
        val))
        << err;
    EXPECT_EQ(oid, "1.3.6.1.2.1.1.1.0");
    EXPECT_EQ(val.type, SnmpValue::Type::Integer);
    EXPECT_EQ(val.intValue, 42);
}

// 1.2 VarBind с OCTET STRING
TEST_F(BerReaderVarBindTest, VarBind_OctetString) {
    snmp::Oid oid;
    SnmpValue val;
    ASSERT_TRUE(encodeAndReadSingle(
        "1.3.6.1.2.1.1.5.0",
        [](BerWriter& w) {
            const char* txt = "UPS";
            w.putTag(0x04);
            w.putLength(3);
            w.putBytes(reinterpret_cast<const uint8_t*>(txt), 3);
        },
        oid,
        val))
        << err;
    EXPECT_EQ(oid, "1.3.6.1.2.1.1.5.0");
    EXPECT_EQ(val.type, SnmpValue::Type::String);
    EXPECT_EQ(val.strValue, "UPS");
}

// 1.3 VarBind с NULL
TEST_F(BerReaderVarBindTest, VarBind_Null) {
    snmp::Oid oid;
    SnmpValue val;
    ASSERT_TRUE(encodeAndReadSingle(
        "1.3.6.1.2.1.1.7.0",
        [](BerWriter& w) {
            w.putTag(0x05);
            w.putLength(0);
        },
        oid,
        val))
        << err;
    EXPECT_EQ(oid, "1.3.6.1.2.1.1.7.0");
    EXPECT_EQ(val.type, SnmpValue::Type::Null);
}

// ============================================================
// Part 2 — Error cases
// ============================================================

// 2.1 VarBind без value
TEST_F(BerReaderVarBindTest, VarBind_MissingValue_Error) {
    buf.clear();
    err.clear();
    BerWriter w(buf);
    size_t vbStart = w.beginSequence(0x30);
    snmp::ber::encodeOid(w, "1.3.6.1.2.1.1.1.0");
    w.endSequence(vbStart);
    const uint8_t* p = buf.data();
    const uint8_t* end = buf.data() + buf.size();
    const uint8_t* vbEnd = nullptr;
    ASSERT_TRUE(BerReader::readSequence(p, end, vbEnd, err));
    snmp::Oid oid;
    ASSERT_TRUE(BerReader::readOid(p, vbEnd, oid, err));
    ASSERT_FALSE(p < vbEnd);
}

// 2.2 VarBind с неподдерживаемым типом значения
TEST_F(BerReaderVarBindTest, VarBind_UnsupportedValueTag_Error) {
    snmp::Oid oid;
    SnmpValue val;
    ASSERT_FALSE(encodeAndReadSingle(
        "1.3.6.1.2.1.1.1.0",
        [](BerWriter& w) {
            w.putTag(0x06);  // invalid value tag (OID)
            w.putLength(1);
            w.putByte(0x2B);
        },
        oid,
        val));
    EXPECT_EQ(err, "Unsupported VarBind value tag");
}
