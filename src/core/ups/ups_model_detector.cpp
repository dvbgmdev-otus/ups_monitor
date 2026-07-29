/**
 * @file ups_model_detector.cpp
 * @ingroup ups
 * @brief Реализация определения модели UPS по SNMP.
 */
#include "ups_model_detector.h"

#include "ini_section_reader.h"
#include "snmp_client_iface.h"
#include "snmp_codec_types.h"
#include "ups_model_spec.h"

namespace ups {
namespace {

/**
 * @brief Добавляет причину отклонения секции при определении модели UPS.
 * @param errors [in/out] Накопленные причины ошибок определения модели.
 * @param section Имя отклонённой секции модели.
 * @param reason Причина отклонения секции.
 */
void appendError(ErrorMessage& errors, const IniSectionName& section, const std::string& reason) {
    if (!errors.empty()) {
        errors += '\n';
    }
    errors += "section [" + section + "]: " + reason;
}

}  // namespace

bool UpsModelDetector::detect(snmp::ISnmpClient& client,
                              const std::string& iniFile,
                              IniSectionName& outModel,
                              ErrorMessage& err) {
    outModel.clear();
    err.clear();
    ErrorMessage detectionErrors;

    // 1. Читаем список секций
    utils::IniSectionReader reader(iniFile);
    if (!reader.ok()) {
        err = reader.lastError();
        return false;
    }

    // 2. Перебираем секции
    for (const auto& section : reader.sections()) {
        // Загружаем спецификацию модели из секции
        UpsModelSpec spec;
        if (!spec.load(iniFile, section)) {
            appendError(detectionErrors, section, "invalid specification: " + spec.lastError());
            continue;
        }

        const std::string& expectedName = spec.modelName();
        const snmp::Oid& nameOid = spec.modelNameOid();

        // делаем SNMP GET
        snmp::codec::SnmpValue value;
        ErrorMessage requestError;
        if (!client.get(nameOid, value, &requestError)) {
            appendError(detectionErrors,
                        section,
                        requestError.empty() ? "SNMP request failed" : requestError);
            continue;
        }

        // ожидаем строковый ответ
        if (value.type != snmp::codec::SnmpValue::Type::String) {
            appendError(detectionErrors, section, "SNMP response is not a string");
            continue;
        }

        if (value.strValue.find(expectedName) != std::string::npos) {
            outModel = section;
            return true;
        }

        appendError(detectionErrors, section, "model name does not match");
    }

    // 3. если дошли сюда, значит ни одна секция не подошла
    err = "UPS model could not be detected";
    if (!detectionErrors.empty()) {
        err += '\n';
        err += detectionErrors;
    }

    return false;
}

}  // namespace ups
