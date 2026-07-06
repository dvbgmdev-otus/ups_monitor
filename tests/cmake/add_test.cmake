# @file tests/cmake/add_test.cmake
# ===== Функция добавления тестов =====
function(add_unit_test TEST_NAME)
    # Функция принимает именованные секции:
    #   SOURCES   - список исходных файлов теста
    #   LIBRARIES - дополнительные библиотеки для линковки
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES LIBRARIES)

    # Разбираем аргументы вызова в переменные UNIT_TEST_SOURCES
    # и UNIT_TEST_LIBRARIES.
    cmake_parse_arguments(UNIT_TEST
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    # Тест без исходных файлов собрать нельзя, поэтому это ошибка
    # конфигурации, а не значение по умолчанию.
    if(NOT UNIT_TEST_SOURCES)
        message(FATAL_ERROR "add_unit_test(${TEST_NAME}): SOURCES is required")
    endif()

    # Большинство unit-тестов проверяет core, поэтому используем его
    # как зависимость по умолчанию. Особые тесты могут передать LIBRARIES.
    if(NOT UNIT_TEST_LIBRARIES)
        set(UNIT_TEST_LIBRARIES ${CORE_NAME})
    endif()

    add_executable(${TEST_NAME}
        ${UNIT_TEST_SOURCES}
    )

    target_link_libraries(${TEST_NAME} PRIVATE
        ${UNIT_TEST_LIBRARIES}
        GTest::gtest_main
    )

    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR}
    )

    # Общие настройки компиляции и линковки держим в одном месте:
    # warnings, coverage flags, debug definitions.
    configure_compile_target(${TEST_NAME})
    configure_link_target(${TEST_NAME})

    # add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    gtest_discover_tests(${TEST_NAME})

endfunction()
