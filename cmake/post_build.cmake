# ==========================================
# @file /cmake/post_build.cmake
# Post build actions
# ==========================================

set(RUNTIME_CONFIG_DIR "${CMAKE_BINARY_DIR}/config")
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD

    # --- config directory ---
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${RUNTIME_CONFIG_DIR}"

    # --- copy config files (one by one for legacy CMake) ---
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/tests/data/config/config.ini
            "${RUNTIME_CONFIG_DIR}/config.ini"

    COMMENT "Prepare runtime directories and config"
)

