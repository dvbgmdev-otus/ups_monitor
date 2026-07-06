# ==========================================
# @file src/core/CMakeLists.txt
# Build core library
# ==========================================

set(CORE_SRC ${CORE_NAME}_SRC)

set(${CORE_SRC}

)

if(${CORE_SRC})
    add_library(${CORE_NAME} ${${CORE_SRC}})

    set_target_properties(${CORE_NAME} PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    target_include_directories(${CORE_NAME} PUBLIC
        ${PROJECT_INCLUDE_DIR}
    )

    configure_compile_target(${CORE_NAME})
else()
    add_library(${CORE_NAME} INTERFACE)

    target_include_directories(${CORE_NAME} INTERFACE
        ${PROJECT_INCLUDE_DIR}
    )

    if(ENABLE_DEBUG_LOG)
        target_compile_definitions(${CORE_NAME} INTERFACE ENABLE_DEBUG_LOG)
    endif()
endif()
