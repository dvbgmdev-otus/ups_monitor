/**
 * @file ups_model_detector.cpp
 * @ingroup ups
 * @brief Реализация определения модели UPS по SNMP.
 */
#include "ups_model_detector.h"

#include "ini_section_reader.h"
#include "snmp_codec_types.h"
#include "ups_model_spec.h"

namespace ups {

bool UpsModelDetector::detect(snmp::ISnmpClient& client,
                              const std::string& iniFile,
                              std::string& outModel,
                              ErrorMessage& err) {
    outModel.clear();
    err.clear();
    ErrorMessage snmpErr;

    // 1. Читаем список секций
    utils::IniSectionReader reader(iniFile);
    if (!reader.ok()) {
        err = reader.lastError();
        return false;
    }

    // 2. Перебираем секции
    for (const auto& section : reader.sections()) {
        UpsModelSpec spec;
        if (!spec.load(iniFile, section)) {
            // некорректная секция — пропускаем
            continue;
        }

        const std::string& expectedName = spec.modelName();
        const snmp::Oid& nameOid = spec.modelNameOid();

        if (expectedName.empty() || nameOid.empty()) continue;

        // делаем SNMP GET
        snmp::codec::SnmpValue value;
        if (!client.get(nameOid, value, &snmpErr)) continue;

        // ожидаем строковый ответ
        if (value.type != snmp::codec::SnmpValue::Type::String) continue;

        if (value.strValue.find(expectedName) != std::string::npos) {
            outModel = section;
            return true;
        }
    }

    // 3. если дошли сюда, значит ни одна секция не подошла
    if (!snmpErr.empty()) {
        err = snmpErr;
    } else {
        err = "UPS model could not be detected";
    }

    return false;
}

}  // namespace ups
