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

add_unit_test(fs_utils_test
    SOURCES
        ${UNIT_TEST_DIR}/utils/fs_utils_test.cpp
    LIBRARIES
        ${UTILS_NAME}
)

add_unit_test(ini_section_reader_test
    SOURCES
        ${UNIT_TEST_DIR}/temp_ini_file.cpp
        ${UNIT_TEST_DIR}/utils/ini_section_reader_test.cpp
    LIBRARIES
        ${UTILS_NAME}
)

add_unit_test(string_utils_test
    SOURCES
        ${UNIT_TEST_DIR}/utils/string_utils_test.cpp
    LIBRARIES
        ${UTILS_NAME}
)

add_unit_test(ups_model_types_test
    SOURCES
        ${UNIT_TEST_DIR}/ups/ups_model_types_test.cpp
    LIBRARIES
        ${CORE_NAME}
)

add_unit_test(ups_param_checker_test
    SOURCES
        ${UNIT_TEST_DIR}/ups/ups_param_checker_test.cpp
    LIBRARIES
        ${CORE_NAME}
)

add_unit_test(ups_model_spec_test
    SOURCES
        ${UNIT_TEST_DIR}/temp_ini_file.cpp
        ${UNIT_TEST_DIR}/ups/ups_model_spec_real_ini_test.cpp
        ${UNIT_TEST_DIR}/ups/ups_model_spec_test.cpp
        ${UNIT_TEST_DIR}/ups/ups_model_spec_values_test.cpp
    LIBRARIES
        ${CORE_NAME}
)

add_dependencies(ups_model_spec_test prepare_runtime_config)

add_unit_test(ups_model_detector_test
    SOURCES
        ${UNIT_TEST_DIR}/temp_ini_file.cpp
        ${UNIT_TEST_DIR}/ups/ups_model_detector_test.cpp
    LIBRARIES
        ${CORE_NAME}
)

add_unit_test(ups_state_poller_test
    SOURCES
        ${UNIT_TEST_DIR}/temp_ini_file.cpp
        ${UNIT_TEST_DIR}/ups/ups_state_poller_test.cpp
    LIBRARIES
        ${CORE_NAME}
)

add_unit_test(ups_state_buffer_test
    SOURCES
        ${UNIT_TEST_DIR}/ups/ups_state_buffer_test.cpp
    LIBRARIES
        ${CORE_NAME}
)

add_unit_test(snmp_ber_test
    SOURCES
        ${UNIT_TEST_DIR}/snmp/ber/encode_primitives_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/integer_reader_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/octet_string_reader_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/oid_reader_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/oid_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/read_tag_and_length_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/sequence_reader_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/skip_value_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/varbind_reader_test.cpp
        ${UNIT_TEST_DIR}/snmp/ber/writer_test.cpp
    LIBRARIES
        ${SNMP_NAME}
)

add_unit_test(snmp_codec_test
    SOURCES
        ${UNIT_TEST_DIR}/snmp/codec/get_request_encode_test.cpp
        ${UNIT_TEST_DIR}/snmp/codec/get_response_decode_errors_test.cpp
        ${UNIT_TEST_DIR}/snmp/codec/get_response_decode_test.cpp
    LIBRARIES
        ${SNMP_NAME}
)

add_unit_test(snmp_client_test
    SOURCES
        ${UNIT_TEST_DIR}/snmp/client/open_socket_test.cpp
    LIBRARIES
        ${SNMP_NAME}
)

set_target_properties(snmp_client_test PROPERTIES
    LINK_FLAGS
        "-Wl,--wrap=socket -Wl,--wrap=setsockopt -Wl,--wrap=close -Wl,--wrap=sendto -Wl,--wrap=recvfrom"
)

if(BUILD_INTEGRATION_TESTS)
    add_subdirectory(integration)
endif()

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
        COMMAND ${SELF_CHECK_SCRIPT}
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
