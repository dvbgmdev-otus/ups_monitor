/**
 * @file ups_model_detector.h
 * @ingroup ups
 * @brief Определение модели UPS по SNMP и INI-спецификации.
 */
#ifndef UPS_MODEL_DETECTOR_H
#define UPS_MODEL_DETECTOR_H

#include <string>

#include "ups_model_types.h"

namespace snmp {
class ISnmpClient;
}

namespace ups {

/**
 * @brief Определяет модель ИБП по SNMP.
 *
 * Алгоритм:
 *  - перебирает секции INI-файла спецификаций;
 *  - для каждой секции читает modelName и modelName.oid;
 *  - выполняет SNMP GET по modelName.oid;
 *  - сравнивает полученное значение с modelName;
 *  - при совпадении возвращает имя секции как идентификатор модели.
 * Если ни одна секция не подошла, возвращает ошибку.
 */
class UpsModelDetector {
public:
    /**
     * @brief Определяет модель ИБП.
     *
     * @param client    Инициализированный SNMP-клиент.
     * @param iniFile   Путь к INI-файлу со спецификациями моделей.
     * @param outModel  [out] Идентификатор модели — имя секции, например "APC_RT_2000_XL".
     * @param err       [out] Текст ошибки при неудаче.
     *
     * @return true, если идентификатор модели успешно определён.
     */
    static bool detect(snmp::ISnmpClient& client,
                       const std::string& iniFile,
                       IniSectionName& outModel,
                       ErrorMessage& err);
};

}  // namespace ups

#endif  // UPS_MODEL_DETECTOR_H
