# ==========================================
# @file /coverage/CMakeLists.txt
# Coverage subsystem
# ==========================================

message(STATUS "")
message(STATUS ">>>>> CONFIGURING COVERAGE SUBSYSTEM <<<<<")

# --- Создаем список файлов для coverage ---
set(COVERAGE_LIST_FILE "${CMAKE_BINARY_DIR}/coverage_file_list.txt")
file(WRITE "${COVERAGE_LIST_FILE}" "")
foreach(f ${COVERAGE_SRC})
    get_filename_component(abs_f "${f}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
    file(APPEND "${COVERAGE_LIST_FILE}" "${abs_f}\n")
endforeach()
message(STATUS "  Coverage file list: ${COVERAGE_LIST_FILE}")

# --- Генерация coverage-скрипта ---
set(COVERAGE_SCRIPT_NAME "run_coverage.sh")
set(COVERAGE_SCRIPT_IN   "${CMAKE_CURRENT_SOURCE_DIR}/${COVERAGE_SCRIPT_NAME}.in")
set(COVERAGE_SCRIPT      "${TEST_BIN_DIR}/${COVERAGE_SCRIPT_NAME}")
configure_file(
    ${COVERAGE_SCRIPT_IN}
    ${COVERAGE_SCRIPT}
    @ONLY
)
file(CHMOD ${COVERAGE_SCRIPT}
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
)
message(STATUS "  Coverage script generated: ${COVERAGE_SCRIPT}")
