/**
 * @file snmp_ber_writer.cpp
 * @ingroup snmp_ber
 * @brief Реализация последовательной сборки ASN.1 BER TLV-структур.
 */
#include "snmp_ber_writer.h"

#include <cstring>

namespace snmp {
namespace ber {

BerWriter::BerWriter(std::vector<uint8_t>& out) : m_out(out) {}

void BerWriter::putTag(uint8_t tag) { putByte(tag); }

void BerWriter::putLength(std::size_t contentLength) {
    // Short form
    if (contentLength < 128) {
        putByte(static_cast<uint8_t>(contentLength));
        return;
    }

    // Long form 1 byte length
    if (contentLength <= 0xFF) {
        putByte(0x81);
        putByte(static_cast<uint8_t>(contentLength));
        return;
    }

    // Long form 2 byte length
    putByte(0x82);
    putByte(static_cast<uint8_t>((contentLength >> 8) & 0xFF));
    putByte(static_cast<uint8_t>(contentLength & 0xFF));
}

void BerWriter::putByte(uint8_t b) { m_out.push_back(b); }

void BerWriter::putBytes(const uint8_t* data, std::size_t len) {
    if (!data || len == 0) return;
    m_out.insert(m_out.end(), data, data + len);
}

std::size_t BerWriter::beginSequence(uint8_t tag) {
    putTag(tag);
    // Мы пока не знаем длину, поэтому ставим placeholder 0x00.
    size_t anchorOffset = m_out.size();
    putByte(0x00);

    return anchorOffset;  // вернём позицию, где нужно будет записать длину
}

void BerWriter::endSequence(size_t anchorOffset) {
    // 1) Длина содержимого SEQUENCE
    size_t contentStart = anchorOffset + 1;
    size_t contentLength = m_out.size() - contentStart;

    // 2) Сгенерируем BER-код длины во временный буфер
    std::vector<uint8_t> lenBuf;
    {
        // Временный writer для длины
        BerWriter tmp(lenBuf);
        tmp.putLength(contentLength);
    }

    // 3) Текущий placeholder занимает 1 байт,
    // но может понадобиться 1, 2 или 3 байта.
    size_t oldLenFieldSize = 1;
    size_t newLenFieldSize = lenBuf.size();

    // 4) Если newLenFieldSize != oldLenFieldSize → нужно сдвинуть данные
    if (newLenFieldSize != oldLenFieldSize) {
        // сдвигаем хвост вправо
        m_out.resize(m_out.size() + (newLenFieldSize - oldLenFieldSize));
        memmove(&m_out[anchorOffset + newLenFieldSize],
                &m_out[anchorOffset + oldLenFieldSize],
                contentLength);
    }

    // 5) Вставляем длину
    for (size_t i = 0; i < newLenFieldSize; i++) m_out[anchorOffset + i] = lenBuf[i];
}

}  // namespace ber
}  // namespace snmp
