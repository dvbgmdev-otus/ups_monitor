/**
 * @file ups_model_spec_real_ini_test.cpp
 * @brief Unit-тесты загрузки runtime-спецификации моделей ИБП.
 */

#include "ups_model_spec_test_base.h"

#include <string>

class UpsModelSpecRealIniTest : public UpsModelSpecTestBase {};

namespace {

const char* const MODEL_SPEC_PATH = "../config/ups_model_spec.ini";

}  // namespace

#if (1)  // Спецификации INELT

// Тест 1.1: Runtime-спецификация INELT MP3000RT успешно загружается
TEST_F(UpsModelSpecRealIniTest, Load_IneltMp3000Rt_Succeeds) {
    expectLoadSuccess(MODEL_SPEC_PATH, "INELT_MP3000RT");

    const auto& parameters = m_spec.parameters();
    EXPECT_FALSE(parameters.empty());
    EXPECT_NE(parameters.find("inputVoltage"), parameters.end());
    EXPECT_NE(parameters.find("outputStatus"), parameters.end());
    EXPECT_NE(parameters.find("batteryStatus"), parameters.end());
}
#endif

#if (1)  // Спецификации APC

// Тест 2.1: Runtime-спецификация APC Smart-UPS RT 2000 XL успешно загружается
TEST_F(UpsModelSpecRealIniTest, Load_ApcRt2000Xl_Succeeds) {
    expectLoadSuccess(MODEL_SPEC_PATH, "APC_RT_2000_XL");

    const auto& parameters = m_spec.parameters();
    EXPECT_FALSE(parameters.empty());
    EXPECT_NE(parameters.find("inputVoltage"), parameters.end());
    EXPECT_NE(parameters.find("outputStatus"), parameters.end());
    EXPECT_NE(parameters.find("batteryStatus"), parameters.end());

    const auto outputStatus = parameters.find("outputStatus");
    ASSERT_NE(outputStatus, parameters.end());
    EXPECT_FALSE(outputStatus->second.bypass.empty());
}
#endif
