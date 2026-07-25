# ==========================================
# @file /cmake/runtime_config.cmake
# Prepare runtime configuration
# ==========================================

set(RUNTIME_CONFIG_SOURCE
    "${CMAKE_SOURCE_DIR}/config/ups_model_spec.ini"
)

set(RUNTIME_CONFIG_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config")

set(RUNTIME_MODEL_SPEC
    "${RUNTIME_CONFIG_DIR}/ups_model_spec.ini"
)

add_custom_command(
    OUTPUT "${RUNTIME_MODEL_SPEC}"

    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${RUNTIME_CONFIG_DIR}"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RUNTIME_CONFIG_SOURCE}"
            "${RUNTIME_MODEL_SPEC}"

    DEPENDS "${RUNTIME_CONFIG_SOURCE}"

    COMMENT "Prepare runtime UPS model specification"
)

add_custom_target(prepare_runtime_config
    DEPENDS "${RUNTIME_MODEL_SPEC}"
)

add_dependencies(${PROJECT_NAME} prepare_runtime_config)
