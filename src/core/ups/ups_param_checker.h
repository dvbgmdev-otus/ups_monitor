/**
 * @file ups_param_checker.h
 * @ingroup ups
 * @brief Проверка параметров UPS по спецификации модели.
 */
#ifndef UPS_PARAM_CHECKER_H
#define UPS_PARAM_CHECKER_H

#include "snmp_codec_types.h"
#include "ups_model_types.h"

namespace ups {

/**
 * @class UpsParamChecker
 * @brief Проверяет один параметр UPS на соответствие допустимым значениям.
 *
 * Класс выполняет:
 *  - проверку валидности полученного SNMP-значения;
 *  - проверку значения на выход за допустимые пределы (normal / bypass);
 *  - установку диагностического флага при отклонении.
 */
class UpsParamChecker {
public:
    /**
     * @brief Проверяет значение параметра UPS на выход за допустимые пределы.
     *
     * @param spec       Спецификация параметра (нормы, bypass и т.д.)
     * @param value      Значение, полученное по SNMP
     * @param deviation  Диагностический бит, который нужно установить
     *                   при выходе параметра за допустимые пределы
     *                   (НЕ может быть UpsDeviationFlags::NONE)
     * @param deviations [in/out] Флаги отклонений состояния UPS
     *
     * @return true  — значение получено и валидно
     * @return false — значение не получено или невалидно
     */
    static bool check(const UpsParamSpec& spec,
                      const snmp::codec::SnmpValue& value,
                      UpsDeviationFlags deviation,
                      UpsDeviationFlags& deviations);
};

}  // namespace ups

#endif  // UPS_PARAM_CHECKER_H
