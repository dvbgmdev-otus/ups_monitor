cmake_minimum_required(VERSION 3.12)

if(1) # PROJECT CONFIG BLOCK

    set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")

    project(ups_monitor VERSION 0.0.${PATCH_VERSION})

    # === Путь для модулей CMake ===
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    # === Путь до исходников ===
    set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
    # Корень покрываемых исходников
    set(COVERAGE_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

    set(PROJECT_INCLUDE_DIR
        ${SRC_DIR}
        ${SRC_DIR}/core
        ${SRC_DIR}/core/utils
        ${SRC_DIR}/main
    )

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    include(compiler_standard)
    include(warnings_config)
    include(build_type_config)
    include(options_config)
    include(target_config)

endif()

if(1) # COVERAGE CONFIG BLOCK
    set(COVERAGE_SRC
        ${SRC_DIR}/main/main.cpp
    )
endif()

if(1) # CORE LIBRARY BLOCK
    set(CORE_NAME ${PROJECT_NAME}_CORE)
    add_subdirectory(${SRC_DIR}/core ${CMAKE_BINARY_DIR}/core)
endif()

if(1) # MAIN APPLICATION BLOCK
    add_subdirectory(${SRC_DIR}/main ${CMAKE_BINARY_DIR}/main)
endif()

if(1) # PROJECT METADATA BLOCK
    # Генерация файла с именем бинарного файла (нужно будет для скриптов)
    file(GENERATE
        OUTPUT "${CMAKE_BINARY_DIR}/project_binary_name.txt"
        CONTENT "${PROJECT_NAME}\n"
    )
endif()

if(1) # ACTIVATE BUILD TESTS BLOCK
    if (BUILD_TESTING)
        enable_testing()
        add_subdirectory(tests)
    endif()
endif()

if(1) # CPACK BLOCK
    set(CPACK_GENERATOR DEB)

    set(CPACK_PACKAGE_NAME ${PROJECT_NAME})

    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

    set(CPACK_PACKAGE_CONTACT "d.v.bogomaz@gmail.com")

    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

    include(CPack)
endif()

include(summary_config)
#include(post_build)
