/**
 * @file ups_model_types.h
 * @ingroup ups
 * @brief Базовые типы доменной модели UPS.
 */
#ifndef UPS_MODEL_TYPES_H
#define UPS_MODEL_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

#include "snmp_codec_types.h"

namespace ups {

using ParamName = std::string;       ///< Имя параметра UPS.
using ErrorMessage = std::string;    ///< Текст ошибки.
using IniSectionName = std::string;  ///< Имя секции ini-файла.

/**
 * @struct NormalValueSpec
 * @brief Описание допустимых значений параметра.
 *
 * Может представлять:
 *  - диапазон значений (min..max)
 *  - набор перечислимых значений
 */
struct NormalValueSpec {
    bool isRange = false;          ///< true, если допустимые значения заданы диапазоном.
    uint32_t min = 0;              ///< Минимум диапазона при isRange == true.
    uint32_t max = 0;              ///< Максимум диапазона при isRange == true.
    std::vector<uint32_t> values;  ///< Допустимые значения при isRange == false.
};

/**
 * @struct UpsParamSpec
 * @brief Описание одного параметра модели UPS.
 */
struct UpsParamSpec {
    ParamName name;                ///< Имя параметра, например inputVoltage или outputStatus.
    snmp::Oid oid;                 ///< SNMP OID параметра.
    NormalValueSpec normal;             ///< Допустимые значения параметра.
    std::vector<uint32_t> bypass;  ///< Значения, соответствующие режиму bypass.
};

/**
 * @enum UpsDeviationFlags
 * @brief Флаги отклонений состояния UPS.
 *
 * Каждый бит описывает конкретную причину ухудшения состояния UPS.
 * Значения используются для формирования битовой маски отклонений.
 */
enum class UpsDeviationFlags : uint32_t {  // NOLINT(performance-enum-size)
    // clang-format off
    NONE           = 0x00000000,  ///< Нет отклонений, все параметры в норме.
    BATTERY_ALERT  = 0x00000001,  ///< Состояние батареи вне нормы.
    CHARGE_ALERT   = 0x00000002,  ///< Недостаточный уровень заряда батареи.
    TEMP_ALERT     = 0x00000004,  ///< Температура батареи вне допустимого диапазона.
    FREQ_ALERT     = 0x00000008,  ///< Частота входного напряжения вне допустимого диапазона.
    INPUT_ALERT    = 0x00000010,  ///< Входное напряжение вне допустимого диапазона.
    OUTPUT_FAILURE = 0x00000020,  ///< Выходное напряжение вне допустимого диапазона.
    BYPASS_ALERT   = 0x00000040,  ///< ИБП работает в режиме байпаса.
    // clang-format on
};

/**
 * @brief Объединяет причины отклонений состояния UPS.
 * @param lhs Левая битовая маска.
 * @param rhs Правая битовая маска.
 * @return Объединённая битовая маска.
 */
UpsDeviationFlags operator|(UpsDeviationFlags lhs, UpsDeviationFlags rhs);

/**
 * @brief Добавляет причины отклонений в битовую маску.
 * @param lhs [in/out] Изменяемая битовая маска.
 * @param rhs Добавляемая битовая маска.
 * @return Ссылка на изменённую битовую маску.
 */
UpsDeviationFlags& operator|=(UpsDeviationFlags& lhs, UpsDeviationFlags rhs);

/**
 * @brief Вычисляет пересечение причин отклонений состояния UPS.
 * @param lhs Левая битовая маска.
 * @param rhs Правая битовая маска.
 * @return Пересечение битовых масок.
 */
UpsDeviationFlags operator&(UpsDeviationFlags lhs, UpsDeviationFlags rhs);

/**
 * @brief Проверяет наличие причины отклонения в битовой маске.
 * @param value Проверяемая битовая маска.
 * @param flag Проверяемая причина отклонения.
 * @return true, если причина присутствует в битовой маске.
 */
bool hasFlag(UpsDeviationFlags value, UpsDeviationFlags flag);

/**
 * @brief Определяет, приводит ли флаг отклонения к аварийному состоянию UPS.
 *
 * @param flag Флаг отклонения.
 * @return true, если отклонение аварийное (Failure).
 * @return false, если отклонение предупреждающее (Warning).
 */
bool isFailureCause(UpsDeviationFlags flag);

/**
 * @brief Возвращает флаг отклонения для указанного параметра UPS.
 *
 * @param paramName Имя параметра (например: inputVoltage).
 * @return Флаг отклонения или UpsDeviationFlags::NONE, если параметр неизвестен.
 */
UpsDeviationFlags toDeviationFlag(const ParamName& paramName);

/**
 * @enum UpsStatus
 * @brief Агрегированное состояние UPS.
 */
enum class UpsStatus {
    OK,       ///< Все параметры успешно получены и соответствуют норме.
    WARNING,  ///< Обнаружено некритичное отклонение или отсутствуют некритичные данные.
    FAILURE,  ///< Критичный параметр вне нормы или его значение не получено.
    NO_INFO   ///< Не получено ни одного ответа от UPS.
};

/**
 * @brief Возвращает строковое представление состояния UPS.
 * @param status Состояние UPS.
 * @return Строковое представление состояния.
 */
const char* toString(UpsStatus status);

/**
 * @struct UpsState
 * @brief Состояние UPS на момент получения данных.
 *
 * Содержит агрегированное состояние и битовую маску отклонений.
 */
struct UpsState {
    UpsStatus status{ UpsStatus::NO_INFO };                   ///< Агрегированное состояние UPS.
    UpsDeviationFlags deviations{ UpsDeviationFlags::NONE };  ///< Битовая маска отклонений.
};

}  // namespace ups

#endif  // UPS_MODEL_TYPES_H
