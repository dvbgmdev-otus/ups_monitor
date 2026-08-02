# @file tests/cmake/add_test.cmake
# ===== Функция добавления unit-тестов =====
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

    # Создаём отдельный исполняемый файл unit-теста.
    add_executable(${TEST_NAME}
        ${UNIT_TEST_SOURCES}
    )

    # Общие fixture и вспомогательные файлы unit-тестов подключаются
    # относительно корневого каталога tests/unit.
    target_include_directories(${TEST_NAME} PRIVATE
        ${UNIT_TEST_DIR}
    )

    # Подключаем проверяемые библиотеки и точку входа GoogleTest.
    target_link_libraries(${TEST_NAME} PRIVATE
        ${UNIT_TEST_LIBRARIES}
        GTest::gtest_main
    )

    # Все тестовые бинарники размещаются в общем runtime-каталоге тестов.
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR}
    )

    # Общие настройки компиляции и линковки держим в одном месте:
    # warnings, coverage flags, debug definitions.
    configure_compile_target(${TEST_NAME})
    configure_link_target(${TEST_NAME})

    # Регистрируем найденные GoogleTest-сценарии как тесты CTest.
    gtest_discover_tests(${TEST_NAME})

endfunction()

# ===== Функция добавления интеграционных тестов =====
function(add_integration_test TEST_NAME)
    # Функция принимает именованные секции:
    #   SOURCES        - список исходных файлов теста
    #   LIBRARIES      - дополнительные библиотеки для линковки
    #   DEPENDENCIES   - зависимости CMake-target
    #   REQUIRED_FILES - дополнительные runtime-файлы теста
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES LIBRARIES DEPENDENCIES REQUIRED_FILES)

    # Разбираем аргументы вызова в переменные INTEGRATION_TEST_SOURCES,
    # INTEGRATION_TEST_LIBRARIES, INTEGRATION_TEST_DEPENDENCIES и
    # INTEGRATION_TEST_REQUIRED_FILES.
    cmake_parse_arguments(INTEGRATION_TEST
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    # Интеграционный тест без исходных файлов собрать нельзя, поэтому
    # отсутствие SOURCES является ошибкой конфигурации.
    if(NOT INTEGRATION_TEST_SOURCES)
        message(FATAL_ERROR "add_integration_test(${TEST_NAME}): SOURCES is required")
    endif()

    # Все интеграционные тесты используют подготовленный runtime UPS Emulator.
    # Путь должен быть задан вызывающим tests/integration/CMakeLists.txt.
    if(NOT UPS_EMULATOR_PATH)
        message(FATAL_ERROR "add_integration_test(${TEST_NAME}): UPS_EMULATOR_PATH is required")
    endif()

    # Большинство интеграционных тестов проверяет core, поэтому используем его
    # как зависимость по умолчанию. Особые тесты могут передать LIBRARIES.
    if(NOT INTEGRATION_TEST_LIBRARIES)
        set(INTEGRATION_TEST_LIBRARIES ${CORE_NAME})
    endif()

    # Создаём отдельный исполняемый файл интеграционного теста.
    add_executable(${TEST_NAME}
        ${INTEGRATION_TEST_SOURCES}
    )

    # Вспомогательные файлы интеграционных тестов подключаются относительно
    # каталога, из которого вызвана функция.
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Передаём C++-коду абсолютный путь к исполняемому файлу эмулятора.
    target_compile_definitions(${TEST_NAME} PRIVATE
        UPS_EMULATOR_EXECUTABLE="${UPS_EMULATOR_PATH}"
    )

    # Подключаем проверяемые библиотеки и точку входа GoogleTest.
    target_link_libraries(${TEST_NAME} PRIVATE
        ${INTEGRATION_TEST_LIBRARIES}
        GTest::gtest_main
    )

    # Все тестовые бинарники размещаются в общем runtime-каталоге тестов.
    set_target_properties(${TEST_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR}
    )

    # Дополнительные CMake-target должны быть подготовлены до сборки теста.
    # Например, эта зависимость используется для runtime-конфигурации монитора.
    if(INTEGRATION_TEST_DEPENDENCIES)
        add_dependencies(${TEST_NAME}
            ${INTEGRATION_TEST_DEPENDENCIES}
        )
    endif()

    # Общие настройки компиляции и линковки держим в одном месте:
    # warnings, coverage flags, debug definitions.
    configure_compile_target(${TEST_NAME})
    configure_link_target(${TEST_NAME})

    # Эмулятор обязателен для каждого интеграционного теста. Конкретный тест
    # может добавить собственные runtime-файлы через REQUIRED_FILES.
    set(required_files
        "${UPS_EMULATOR_PATH}"
        ${INTEGRATION_TEST_REQUIRED_FILES}
    )

    # Регистрируем найденные GoogleTest-сценарии как интеграционные тесты CTest.
    # Общая блокировка запрещает одновременно занимать UDP-порт эмулятора,
    # а timeout завершает зависший тест.
    gtest_discover_tests(${TEST_NAME}
        PROPERTIES
            LABELS integration
            REQUIRED_FILES "${required_files}"
            RESOURCE_LOCK snmp_emulator_port_1161
            TIMEOUT 15
    )
endfunction()
