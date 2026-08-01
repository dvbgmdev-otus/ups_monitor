# ==========================================
# @file /cmake/runtime_config.cmake
# Prepare runtime configuration
# ==========================================

set(RUNTIME_CONFIG_SOURCE
    "${CMAKE_SOURCE_DIR}/config/ups_model_spec.ini"
)

function(add_runtime_config target_name runtime_dir)
    set(runtime_config_dir "${runtime_dir}/config")
    set(runtime_model_spec "${runtime_config_dir}/ups_model_spec.ini")

    add_custom_command(
        OUTPUT "${runtime_model_spec}"

        COMMAND ${CMAKE_COMMAND} -E make_directory
                "${runtime_config_dir}"

        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${RUNTIME_CONFIG_SOURCE}"
                "${runtime_model_spec}"

        DEPENDS "${RUNTIME_CONFIG_SOURCE}"

        COMMENT "Prepare runtime UPS model specification for ${target_name}"
    )

    add_custom_target(${target_name}
        DEPENDS "${runtime_model_spec}"
    )
endfunction()
