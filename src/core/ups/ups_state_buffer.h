/**
 * @file ups_state_buffer.h
 * @ingroup ups
 * @brief Потокобезопасный буфер состояния UPS.
 *
 * Класс UpsStateBuffer инкапсулирует:
 *  - хранение последнего состояния UPS;
 *  - потокобезопасный доступ к нему;
 *  - семантику «однократного потребления» состояния.
 */

#ifndef UPS_STATE_BUFFER_H
#define UPS_STATE_BUFFER_H

#include <mutex>

#include "ups_model_types.h"  // UpsState

namespace ups {

/**
 * @class UpsStateBuffer
 * @brief Потокобезопасный буфер состояния UPS.
 */
class UpsStateBuffer {
public:
    /**
     * @brief Создаёт пустой контейнер состояния UPS.
     */
    UpsStateBuffer() = default;

    /**
     * @brief Деструктор.
     */
    ~UpsStateBuffer() = default;

    /**
     * @brief Запрещает копирование контейнера состояния.
     */
    UpsStateBuffer(const UpsStateBuffer&) = delete;

    /**
     * @brief Запрещает копирующее присваивание контейнера состояния.
     */
    UpsStateBuffer& operator=(const UpsStateBuffer&) = delete;

    /**
     * @brief Сохраняет новое состояние UPS.
     *
     * Перезаписывает предыдущее состояние (если оно было),
     * помечая его как доступный для обработки.
     *
     * Потокобезопасно.
     *
     * @param state Состояние UPS.
     */
    void storeState(const UpsState& state);

    /**
     * @brief Пытается извлечь и «потребить» состояние.
     *
     * Если состояние присутствует — копирует его в out,
     * сбрасывает внутренний флаг и возвращает true.
     *
     * Если состояния нет — ничего не делает и возвращает false.
     *
     * Потокобезопасно.
     *
     * @param out [out] Извлечённое состояние.
     * @return true, если состояние было получено; false иначе.
     */
    bool tryConsumeState(UpsState& out);

private:
    std::mutex m_mutex;          ///< Мьютекс доступа к состоянию.
    bool m_hasState{ false };    ///< Признак наличия непотреблённого состояния.
    UpsState m_state;            ///< Последнее полученное состояние UPS.
};

}  // namespace ups

#endif  // UPS_STATE_BUFFER_H
