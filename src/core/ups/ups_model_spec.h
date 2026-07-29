/**
 * @file ups_model_spec.h
 * @ingroup ups
 * @brief Спецификация модели UPS и правила её загрузки.
 */
#ifndef UPS_MODEL_SPEC_H
#define UPS_MODEL_SPEC_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "ups_model_types.h"

namespace ups {

/**
 * @class UpsModelSpec
 * @brief Спецификация модели UPS.
 *
 * Содержит описание параметров модели UPS,
 * загружаемое из ini-файла.
 */
class UpsModelSpec {
public:
    /**
     * @brief Загружает спецификацию модели UPS из ini-файла.
     *
     * @param path Путь к ini-файлу.
     * @param section Имя секции (ключ модели).
     * @return true при успешной загрузке и валидации.
     *
     * @warning Если метод вернул false, спецификация может быть заполнена частично.
     * В этом случае разрешено использовать только lastError().
     */
    bool load(const std::string& path, const IniSectionName& section);

    /**
     * @brief Возвращает SNMP OID имени модели UPS.
     * @return SNMP OID имени модели UPS.
     */
    const snmp::Oid& modelNameOid() const;

    /**
     * @brief Возвращает имя модели UPS.
     * @return Имя модели UPS.
     */
    const std::string& modelName() const;

    /**
     * @brief Возвращает параметры модели, индексированные по имени.
     * @return Параметры модели UPS.
     */
    const std::map<ParamName, UpsParamSpec>& parameters() const;

    /**
     * @brief Возвращает текст последней ошибки загрузки или валидации.
     * @return Текст последней ошибки.
     */
    const ErrorMessage& lastError() const;

private:
    std::string m_modelName{};                    ///< Имя модели UPS.
    snmp::Oid m_modelNameOid{};                   ///< SNMP OID имени модели UPS.
    std::map<ParamName, UpsParamSpec> m_parameters;  ///< Параметры модели по имени.
    ErrorMessage m_lastError{};                   ///< Последняя ошибка загрузки или валидации.

    /**
     * @brief Разбирает описание допустимых значений параметра.
     * @param value Строковое значение поля normal.
     * @param out [out] Разобранное описание допустимых значений.
     * @param error [out] Текст ошибки разбора.
     * @return true, если значение успешно разобрано.
     */
    static bool parseNormal(const std::string& value, NormalValueSpec& out, ErrorMessage& error);

    /**
     * @brief Разбирает диапазон значений формата min..max.
     * @param s Строковое описание диапазона.
     * @param min [out] Минимальное значение диапазона.
     * @param max [out] Максимальное значение диапазона.
     * @param error [out] Текст ошибки разбора.
     * @return true, если диапазон успешно разобран.
     */
    static bool parseRange(const std::string& s,
                           uint32_t& min,
                           uint32_t& max,
                           ErrorMessage& error);

    /**
     * @brief Разбирает список перечислимых значений.
     * @param s Строковое описание значений через запятую.
     * @param out [out] Разобранные значения.
     * @param error [out] Текст ошибки разбора.
     * @return true, если список успешно разобран.
     */
    static bool parseEnumValues(const std::string& s,
                                std::vector<uint32_t>& out,
                                ErrorMessage& error);

    /**
     * @brief Проверяет корректность загруженной спецификации.
     * @return true, если спецификация корректна.
     */
    bool validate();
};

}  // namespace ups

#endif  // UPS_MODEL_SPEC_H
