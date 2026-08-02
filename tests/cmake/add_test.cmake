# @file tests/cmake/add_test.cmake

# ===== Внутренняя функция добавления GTest-target =====
function(_add_gtest_target TEST_NAME)
    # Булевы опции helper не принимает.
    set(options)

    # Эти именованные аргументы принимают по одному значению.
    set(oneValueArgs
        INCLUDE_DIR
        LABEL
        RESOURCE_LOCK
        TIMEOUT
    )

    # Эти именованные аргументы принимают списки значений.
    set(multiValueArgs
        SOURCES
        LIBRARIES
        DEFINITIONS
        DEPENDENCIES
        COMMON_REQUIRED_FILES
        REQUIRED_FILES
    )

    # Разбираем все общие аргументы unit- и интеграционных тестов
    # в переменные с префиксом TEST_.
    cmake_parse_arguments(TEST
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    # Тест без исходных файлов собрать нельзя, поэтому отсутствие SOURCES
    # является ошибкой конфигурации.
    if(NOT TEST_SOURCES)
        message(FATAL_ERROR "Test ${TEST_NAME}: SOURCES is required")
    endif()

    # Каждый тест должен явно определить каталог вспомогательных заголовков.
    if(NOT TEST_INCLUDE_DIR)
        message(FATAL_ERROR "Test ${TEST_NAME}: INCLUDE_DIR is required")
    endif()

    # Большинство тестов проверяет core, поэтому используем его как зависимость
    # по умолчанию. Особые тесты могут передать LIBRARIES.
    if(NOT TEST_LIBRARIES)
        set(TEST_LIBRARIES ${CORE_NAME})
    endif()

    # Создаём отдельный исполняемый файл теста.
    add_executable(${TEST_NAME}
        ${TEST_SOURCES}
    )

    # Подключаем fixture и вспомогательные заголовки теста.
    target_include_directories(${TEST_NAME} PRIVATE
        ${TEST_INCLUDE_DIR}
    )

    # Передаём необязательные определения препроцессора.
    if(TEST_DEFINITIONS)
        target_compile_definitions(${TEST_NAME} PRIVATE
            ${TEST_DEFINITIONS}
        )
    endif()

    # Подключаем проверяемые библиотеки и точку входа GoogleTest.
    target_link_libraries(${TEST_NAME} PRIVATE
        ${TEST_LIBRARIES}
        GTest::gtest_main
    )

    # Все тестовые бинарники размещаются в общем runtime-каталоге тестов.
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR}
    )

    # Дополнительные CMake-target должны быть подготовлены до сборки теста.
    if(TEST_DEPENDENCIES)
        add_dependencies(${TEST_NAME}
            ${TEST_DEPENDENCIES}
        )
    endif()

    # Общие настройки компиляции и линковки держим в одном месте:
    # warnings, coverage flags, debug definitions.
    configure_compile_target(${TEST_NAME})
    configure_link_target(${TEST_NAME})

    # Объединяем обязательные файлы вида тестов и конкретного сценария.
    set(required_files
        ${TEST_COMMON_REQUIRED_FILES}
        ${TEST_REQUIRED_FILES}
    )

    # Регистрируем найденные GoogleTest-сценарии как тесты CTest.
    # Расширенные свойства добавляются только когда они были переданы.
    if(TEST_LABEL OR required_files OR TEST_RESOURCE_LOCK OR TEST_TIMEOUT)
        gtest_discover_tests(${TEST_NAME}
            PROPERTIES
                LABELS "${TEST_LABEL}"
                REQUIRED_FILES "${required_files}"
                RESOURCE_LOCK "${TEST_RESOURCE_LOCK}"
                TIMEOUT "${TEST_TIMEOUT}"
        )
    else()
        gtest_discover_tests(${TEST_NAME})
    endif()
endfunction()

# ===== Функция добавления unit-тестов =====
function(add_unit_test TEST_NAME)
    # Unit-тесты используют общий каталог fixture и вспомогательных заголовков.
    # Остальные аргументы без повторного разбора передаются общему helper.
    _add_gtest_target(${TEST_NAME}
        INCLUDE_DIR
            "${UNIT_TEST_DIR}"
        ${ARGN}
    )
endfunction()

# ===== Функция добавления интеграционных тестов =====
function(add_integration_test TEST_NAME)
    # Все интеграционные тесты используют подготовленный runtime UPS Emulator.
    # Путь должен быть задан вызывающим tests/integration/CMakeLists.txt.
    if(NOT UPS_EMULATOR_PATH)
        message(FATAL_ERROR
            "add_integration_test(${TEST_NAME}): UPS_EMULATOR_PATH is required"
        )
    endif()

    # Интеграционные тесты получают общие настройки эмулятора и CTest.
    # Аргументы конкретного теста без повторного разбора передаются helper.
    _add_gtest_target(${TEST_NAME}
        INCLUDE_DIR
            "${CMAKE_CURRENT_SOURCE_DIR}"
        DEFINITIONS
            UPS_EMULATOR_EXECUTABLE="${UPS_EMULATOR_PATH}"
        COMMON_REQUIRED_FILES
            "${UPS_EMULATOR_PATH}"
        LABEL
            integration
        RESOURCE_LOCK
            snmp_emulator_port_1161
        TIMEOUT
            15
        ${ARGN}
    )
endfunction()
