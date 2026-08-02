# ==========================================
# @file tests/integration/CMakeLists.txt
# Build integration tests
# ==========================================

set(UPS_EMULATOR_PATH
    "${TEST_BIN_DIR}/ups_emulator/ups_emulator"
)

add_integration_test(snmp_client_integration_test
    SOURCES
        snmp_client_integration_test.cpp
        snmp_ready_waiter.cpp
        ups_emulator_process.cpp
    LIBRARIES
        ${SNMP_NAME}
)

add_integration_test(ups_monitor_integration_test
    SOURCES
        ups_monitor_integration_test.cpp
        ups_emulator_process.cpp
    LIBRARIES
        ${CORE_NAME}
    DEPENDENCIES
        prepare_test_runtime_config
    REQUIRED_FILES
        "${TEST_BIN_DIR}/config/ups_model_spec.ini"
)
