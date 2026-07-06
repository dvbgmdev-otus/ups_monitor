# ==========================================
# @file /cmake/compiler_standard.cmake
# Compiler standard configuration
# ==========================================

# ============================================================================
#  compiler_standard.cmake
#
#  Назначение:
#    Гарантированно зафиксировать стандарт C++11 для проекта
#    с поддержкой legacy CMake (2.8.x) и современных версий.
#
#  Причина:
#    - Astra Linux (CMake 2.8.x) по умолчанию компилирует в C++98
#    - Проект использует возможности C++11 (std::thread, auto, constexpr и т.д.)
#
# ============================================================================

message(STATUS "")
message(STATUS ">>>>> CONFIGURING C++ LANGUAGE STANDARD <<<<<")

if (CMAKE_VERSION VERSION_LESS "3.1")
    # Legacy CMake (e.g. 2.8.12 on Astra Linux)
    message(STATUS "  Legacy CMake detected (${CMAKE_VERSION})")
    message(STATUS "  Forcing C++11 via compiler flags")
    add_definitions(-std=gnu++11)
    # PROJECT_CXX_STANDARD_INFO - переменная, что бы потом выводить в summary
    set(PROJECT_CXX_STANDARD_INFO "C++11 (forced via -std=gnu++11)")
else()
    # Modern CMake
    message(STATUS "  Modern CMake detected (${CMAKE_VERSION})")
    message(STATUS "  Using CMake CXX_STANDARD mechanism")
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(PROJECT_CXX_STANDARD_INFO "C++${CMAKE_CXX_STANDARD} (CMake native)")
endif()
