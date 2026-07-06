# ==========================================
# @file /cmake/options_config.cmake
# Project build options (tests, coverage)
# ==========================================

message(STATUS "")
message(STATUS ">>>>> CONFIGURING PROJECT OPTIONS <<<<<")

# ---- Отключение тестов и покрытия для старых версий CMake (< 3.13) ----
# На Astra Linux используется устаревший CMake, не поддерживающий target_link_options()
if(CMAKE_VERSION VERSION_LESS 3.13)
    message(STATUS "  Legacy CMake detected (${CMAKE_VERSION})")
    message(STATUS "  Disabling tests and coverage")
    set(BUILD_TESTING OFF CACHE BOOL "Build unit tests" FORCE)
    set(BUILD_COVERAGE OFF CACHE BOOL "Enable coverage instrumentation" FORCE)
else()
    option(BUILD_TESTING "Build unit tests" ON)
    option(BUILD_COVERAGE "Enable coverage instrumentation" OFF)
endif()

# ---- Покрытие кода возможно только при включённых тестах ----
if(BUILD_COVERAGE)
    set(BUILD_TESTING ON CACHE BOOL "Build unit tests" FORCE)
    set(COVERAGE_COMPILE_FLAGS "--coverage;-O0;-g")
    set(COVERAGE_LINK_FLAGS "--coverage")
endif()

message(STATUS "  BUILD_TESTING = ${BUILD_TESTING}")
message(STATUS "  BUILD_COVERAGE = ${BUILD_COVERAGE}")

option(ENABLE_DEBUG_LOG "Enable debug logs" ON)
message(STATUS "  ENABLE_DEBUG_LOG = ${ENABLE_DEBUG_LOG}")
