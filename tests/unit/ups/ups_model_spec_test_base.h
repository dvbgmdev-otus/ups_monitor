/**
 * @file ups_model_spec_test_base.h
 * @brief Общая fixture для unit-тестов ups::UpsModelSpec.
 */

#pragma once

#include "temp_ini_file.h"
#include "ups_model_spec.h"

#include <gtest/gtest.h>

#include <string>

class UpsModelSpecTestBase : public ::testing::Test {
protected:
    ups::UpsModelSpec m_spec;
    test::TempIniFileStorage m_tempIniFiles;

    void expectLoadFailure(const std::string& file,
                           const std::string& section,
                           const std::string& expected = "") {
        const bool ok = m_spec.load(file, section);
        EXPECT_FALSE(ok) << m_spec.lastError();
        EXPECT_FALSE(m_spec.lastError().empty()) << "Error message must not be empty";
        if (!expected.empty()) {
            EXPECT_NE(m_spec.lastError().find(expected), std::string::npos) << m_spec.lastError();
        }
    }

    void expectLoadSuccess(const std::string& file, const std::string& section) {
        const bool ok = m_spec.load(file, section);
        EXPECT_TRUE(ok) << m_spec.lastError();
        EXPECT_TRUE(m_spec.lastError().empty()) << m_spec.lastError();
    }
};
