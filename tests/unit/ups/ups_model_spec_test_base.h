/**
 * @file ups_model_spec_test_base.h
 * @brief Общая fixture для unit-тестов ups::UpsModelSpec.
 */

#pragma once

#include <gtest/gtest.h>

#include <string>

#include "temp_ini_file.h"
#include "ups_model_spec.h"

class UpsModelSpecTestBase : public ::testing::Test {
protected:
    ups::UpsModelSpec m_spec;
    test::TempIniFileStorage m_tempIniFiles;

    /**
     * @brief Проверяет, что загрузка спецификации завершилась ошибкой.
     * @param file Путь к INI-файлу спецификаций.
     * @param section Имя загружаемой секции модели.
     * @param expected Ожидаемая подстрока сообщения об ошибке.
     */
    void expectLoadFailure(const std::string& file,
                           const std::string& section,
                           const std::string& expected = "") {
        const bool ok = m_spec.load(file, section);
        ASSERT_FALSE(ok) << "Expected loading section '" << section << "' to fail";
        ASSERT_FALSE(m_spec.lastError().empty()) << "Error message must not be empty";
        if (!expected.empty()) {
            EXPECT_NE(m_spec.lastError().find(expected), std::string::npos) << m_spec.lastError();
        }
    }

    /**
     * @brief Проверяет, что спецификация успешно загружена без сообщения об ошибке.
     * @param file Путь к INI-файлу спецификаций.
     * @param section Имя загружаемой секции модели.
     */
    void expectLoadSuccess(const std::string& file, const std::string& section) {
        const bool ok = m_spec.load(file, section);
        ASSERT_TRUE(ok) << m_spec.lastError();
        EXPECT_TRUE(m_spec.lastError().empty()) << m_spec.lastError();
    }
};
