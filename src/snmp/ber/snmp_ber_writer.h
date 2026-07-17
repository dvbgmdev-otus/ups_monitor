/**
 * @file snmp_ber_writer.h
 * @ingroup snmp_ber
 * @brief Низкоуровневый ASN.1 BER encoder.
 *
 * Класс BerWriter предназначен для последовательной сборки
 * ASN.1 BER TLV-структур (Tag-Length-Value).
 *
 * Поддерживает:
 * - запись тегов и значений;
 * - кодирование длины (short и long form);
 * - построение вложенных SEQUENCE / constructed типов.
 *
 * Не содержит логики протоколов верхнего уровня (SNMP)
 * и используется SNMP-кодеком для построения пакетов.
 */
#ifndef SNMP_BER_WRITER_H
#define SNMP_BER_WRITER_H

#include <cstddef>
#include <cstdint>
#include <vector>

namespace snmp {
namespace ber {

/**
 * @class BerWriter
 * @brief Вспомогательный класс для кодирования ASN.1 BER.
 *
 * BerWriter управляет записью BER-структур в пользовательский
 * буфер и обеспечивает корректное формирование вложенных
 * TLV-элементов.
 *
 * Класс не владеет буфером и не хранит состояние протокола.
 */
class BerWriter {
public:
    /**
     * @brief Создаёт BER-writer, записывающий данные в заданный буфер.
     *
     * @param out Внешний буфер, в который будет производиться запись.
     *            Буфер должен существовать на всём протяжении жизни BerWriter.
     */
    explicit BerWriter(std::vector<uint8_t>& out);

    // --------------------------------------------------------
    // Низкоуровневые записи
    // --------------------------------------------------------

    /**
     * @brief Записывает ASN.1 BER тег в выходной буфер.
     *
     * @param tag Тег ASN.1 BER (например, INTEGER = 0x02, SEQUENCE = 0x30)
     */
    void putTag(uint8_t tag);

    /**
     * @brief Записывает ASN.1 BER длину в выходной буфер.
     *
     * Поддерживает как короткий (short form), так и длинный
     * форматы длины.
     *
     * @param contentLength Длина содержимого в байтах.
     */
    void putLength(std::size_t contentLength);

    /**
     * @brief Записывает один байт в выходной буфер.
     *
     * @param b Байт для записи.
     */
    void putByte(uint8_t b);

    /**
     * @brief Записывает произвольный массив байтов в выходной буфер.
     *
     * @param data Указатель на данные для записи.
     * @param len Количество байт для записи.
     */
    void putBytes(const uint8_t* data, std::size_t len);

    // --------------------------------------------------------
    // Работа с SEQUENCE / Constructed TLV
    // --------------------------------------------------------

    /**
     * @brief Начинает constructed ASN.1 элемент (например, SEQUENCE).
     *
     * Записывает тег (TAG) и временный placeholder для длины.
     * Реальная длина будет вычислена при вызове endSequence().
     * placeholder изначально занимает 1 байт, но может быть расширен
     * при необходимости.
     *
     * @param tag ASN.1 тег constructed элемента.
     * @return Смещение в буфере, используемое как якорь. По этому
     *         смещению позже будет записана реальная длина.
     */
    std::size_t beginSequence(uint8_t tag);

    /**
     * @brief Завершает constructed ASN.1 элемент.
     *
     * Вычисляет фактическую длину содержимого и записывает
     * её в ранее сохранённый placeholder.
     *
     * @param anchorOffset Смещение, возвращённое beginSequence().
     */
    void endSequence(std::size_t anchorOffset);

    /**
     * @brief Возвращает текущий размер сформированных BER-данных.
     *
     * @return Размер данных в байтах.
     */
    size_t size() const;

private:
    std::vector<uint8_t>& m_out;  ///< Ссылка на выходной буфер.
};

}  // namespace ber
}  // namespace snmp

#endif  // SNMP_BER_WRITER_H
