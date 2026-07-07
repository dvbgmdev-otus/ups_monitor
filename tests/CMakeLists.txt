# ==========================================
# @file tests/CMakeLists.txt
# Build tests subsystem
# ==========================================

message(STATUS "")
message(STATUS ">>>>> CONFIGURING BUILD TESTS SUBSYSTEM <<<<<")

# --- Подключение вспомогательных CMake-функций для тестов ---
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/add_test.cmake)

# --- Папка для unit-тестов ---
set(UNIT_TEST_DIR "${CMAKE_SOURCE_DIR}/tests/unit")
message(STATUS "  Unit tests dir: ${UNIT_TEST_DIR}")

# --- Папка для тестовых данных ---
# set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data")
# add_compile_definitions(TEST_DATA_DIR=\"${TEST_DATA_DIR}\")
# message(STATUS "  Test data directory: ${TEST_DATA_DIR}")

# --- Папка для бинарников тестов ---
set(BIN_DIR "${CMAKE_BINARY_DIR}/bin")
set(TEST_BIN_DIR "${BIN_DIR}/tests")
message(STATUS "  Test binaries dir: ${TEST_BIN_DIR}")

# --- Подключение GoogleTest ---
include(FetchContent)

set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# --- Добавление тестов ---
add_unit_test(empty_test SOURCES
    ${UNIT_TEST_DIR}/empty_test.cpp
)

if(1) # SELF-CHECK TEST BLOCK
    set(SELF_CHECK_SCRIPT_NAME "self_check.sh")
    set(SELF_CHECK_SCRIPT_IN   "${UNIT_TEST_DIR}/${SELF_CHECK_SCRIPT_NAME}.in")
    set(SELF_CHECK_SCRIPT      "${TEST_BIN_DIR}/${SELF_CHECK_SCRIPT_NAME}")

    configure_file(
        ${SELF_CHECK_SCRIPT_IN}
        ${SELF_CHECK_SCRIPT}
        @ONLY
    )

    file(CHMOD ${SELF_CHECK_SCRIPT}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ GROUP_EXECUTE
                    WORLD_READ WORLD_EXECUTE
    )

    add_test(
        NAME self_check
        COMMAND ${SELF_CHECK_SCRIPT} $<TARGET_FILE:${PROJECT_NAME}>
    )

    message(STATUS "  Self-check script generated: ${SELF_CHECK_SCRIPT}")
endif()

message(STATUS "  GoogleTest added")

# --- Подключение coverage-подсистемы (опционально) ---
if (BUILD_COVERAGE)
    message(STATUS "  Coverage enabled: configuring coverage subsystem")
    add_subdirectory(coverage)
else()
    message(STATUS "  Coverage disabled")
endif()

# --- Копирование конфигурации для unit-тестов ---
# add_custom_target(prepare_runtime_config ALL
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#             "${BIN_DIR}/config"
#
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${FPO_CONFIG_DIR}/ups_model_spec.ini"
#             "${BIN_DIR}/config/ups_model_spec.ini"
#
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different
#             "${FPO_CONFIG_DIR}/config.ini"
#             "${BIN_DIR}/config/config.ini"
#
#     COMMAND ${CMAKE_COMMAND} -E make_directory
#             "${TEST_BIN_DIR}/log"
#
#     COMMENT "Prepare shared runtime config in ${BIN_DIR}/config"
# )
