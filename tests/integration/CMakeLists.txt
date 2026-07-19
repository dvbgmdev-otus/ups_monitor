# ==========================================
# @file tests/integration/CMakeLists.txt
# Build integration tests
# ==========================================

set(UPS_EMULATOR_PATH
    "${TEST_BIN_DIR}/ups_emulator/ups_emulator"
)

add_executable(snmp_client_integration_test
    snmp_client_integration_test.cpp
    snmp_ready_waiter.cpp
    ups_emulator_process.cpp
)

target_include_directories(snmp_client_integration_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(snmp_client_integration_test PRIVATE
    UPS_EMULATOR_EXECUTABLE="${UPS_EMULATOR_PATH}"
)

target_link_libraries(snmp_client_integration_test PRIVATE
    ${SNMP_NAME}
    GTest::gtest_main
)

set_target_properties(snmp_client_integration_test PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR}
)

configure_compile_target(snmp_client_integration_test)
configure_link_target(snmp_client_integration_test)

gtest_discover_tests(snmp_client_integration_test
    PROPERTIES
        LABELS integration
        REQUIRED_FILES "${UPS_EMULATOR_PATH}"
        RESOURCE_LOCK snmp_emulator_port_1161
        TIMEOUT 15
)
