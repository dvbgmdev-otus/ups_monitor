# ==========================================
# @file /cmake/build_type_config.cmake
# Build type configuration module
# Supports single-config and multi-config generators
# ==========================================

# --- Можно указать тип сборки вручную ---
# Пример:
#   cmake -B build -DCMAKE_BUILD_TYPE=Debug
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
#   cmake -B build -DCMAKE_BUILD_TYPE=MinSizeRel
#
# Поддерживаемые типы (single-config):
#   Debug
#   Release
#   RelWithDebInfo
#   MinSizeRel
#
# Для multi-config генераторов (Ninja Multi-Config, Visual Studio)
# активная конфигурация выбирается на этапе сборки:
#   cmake --build build --config Debug

message(STATUS "")
message(STATUS ">>>>> CONFIGURING BUILD TYPE <<<<<")

if(NOT CMAKE_CONFIGURATION_TYPES)

    if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE "Release"
            CACHE STRING "Build type"
            FORCE)
    endif()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug
        Release
        RelWithDebInfo
        MinSizeRel
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "  Generator type:   Multi-config")
    message(STATUS "  Configurations:   ${CMAKE_CONFIGURATION_TYPES}")
    message(STATUS "  Active config:    (selected at build time)")
else()
    message(STATUS "  Generator type:   Single-config")
    message(STATUS "  Build type:       ${CMAKE_BUILD_TYPE}")
endif()
