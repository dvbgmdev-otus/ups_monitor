# ==========================================
# @file /cmake/summary_config.cmake
# Build configuration summary
# ==========================================

message(STATUS "")
message(STATUS "===========================================")
message(STATUS "  Project name:       ${PROJECT_NAME}")

if(DEFINED QT_VER_USED)
    message(STATUS "  Qt version used:    Qt${QT_VER_USED} (${QT_VERSION_STRING})")
endif()

message(STATUS "  CMake version:      ${CMAKE_VERSION}")
message(STATUS "  Compiler:           ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
if(DEFINED PROJECT_CXX_STANDARD_INFO)
    message(STATUS "  C++ standard:       ${PROJECT_CXX_STANDARD_INFO}")
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "  Generator type:     Multi-config")
    message(STATUS "  Configurations:     ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "  Build type:         ${CMAKE_BUILD_TYPE}")
endif()

message(STATUS "  Build testing:      ${BUILD_TESTING}")
message(STATUS "  Coverage enabled:   ${BUILD_COVERAGE}")
message(STATUS "  Debug logs enabled: ${ENABLE_DEBUG_LOG}")

message(STATUS "  Source dir: ${CMAKE_SOURCE_DIR}")
message(STATUS "  Build dir:  ${CMAKE_BINARY_DIR}")

string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")
message(STATUS "  Timestamp:  ${BUILD_TIMESTAMP}")
message(STATUS "===========================================")
message(STATUS "")
message(STATUS "Build \"${PROJECT_NAME}\" complete")
message(STATUS "")
