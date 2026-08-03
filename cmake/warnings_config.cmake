# ==========================================
# @file /cmake/warnings_config.cmake
# Compiler warnings configuration
# ==========================================

message(STATUS "")
message(STATUS ">>>>> CONFIGURING COMPILER WARNINGS <<<<<")

set(WARN_FLAGS
    -Wall
    -Wextra
    -Wswitch-enum
)

# ---- Включение pedantic-предупреждения ----
option(ENABLE_PEDANTIC_WARNINGS "Enable pedantic compiler warnings" ON)
# GCC < 4.8 не поддерживает -Wpedantic, используется -pedantic
if(ENABLE_PEDANTIC_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
            list(APPEND WARN_FLAGS -pedantic)
        else()
            list(APPEND WARN_FLAGS -Wpedantic)
        endif()
    endif()
endif()

message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "  Warnings: ${WARN_FLAGS}")
