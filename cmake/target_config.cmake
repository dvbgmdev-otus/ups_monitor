# ==========================================
# @file /cmake/target_config.cmake
# Target configuration helpers
# ==========================================

function(configure_compile_target TARGET_NAME)
    target_compile_options(${TARGET_NAME} PRIVATE ${WARN_FLAGS})

    if(BUILD_COVERAGE)
        target_compile_options(${TARGET_NAME} PRIVATE ${COVERAGE_COMPILE_FLAGS})
    endif()

    if(ENABLE_DEBUG_LOG)
        target_compile_definitions(${TARGET_NAME} PRIVATE ENABLE_DEBUG_LOG)
    endif()
endfunction()

function(configure_link_target TARGET_NAME)
    if(BUILD_COVERAGE)
        if(CMAKE_VERSION VERSION_GREATER "3.13" OR
           CMAKE_VERSION VERSION_EQUAL "3.13")
            target_link_options(${TARGET_NAME} PRIVATE ${COVERAGE_LINK_FLAGS})
        else()
            set_target_properties(${TARGET_NAME} PROPERTIES
                LINK_FLAGS "${COVERAGE_LINK_FLAGS}"
            )
        endif()
    endif()
endfunction()
