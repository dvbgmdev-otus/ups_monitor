#!/usr/bin/env bash
#
# itest.sh — запуск интеграционных тестов с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → подготавливает эмулятор,
#     проверяет integration-конфигурацию и запускает тесты
#   - Если скрипт выполняется на хосте → запускает выполнение внутри Docker контейнера
#
# Поведение:
#   - Перед тестами собирает runtime UPS-эмулятора
#   - Если integration-сборка отсутствует или имеет другую конфигурацию —
#     запускает scripts/build.sh
#   - После успешной сборки запускает тесты с меткой integration через ctest
#   - Аргументы itest.sh пробрасываются в ctest
#
# Коды возврата:
#   0 — все интеграционные тесты успешно пройдены
#   1 — ошибка подготовки, сборки или запуска тестов

set -eEuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/error_trap.sh"
setup_error_trap
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/docker.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/cmake_cache.sh"

INTEGRATION_TEST_CMAKE_CONFIG=(
    BUILD_TESTING BOOL ON
    BUILD_COVERAGE BOOL OFF
    BUILD_INTEGRATION_TESTS BOOL ON
)

# Внутренний helper: запускает сборку с переданной CMake-конфигурацией.
build_with_cmake_config() {
    local build_args=()

    while (( $# > 0 )); do
        build_args+=("-D$1:$2=$3")
        shift 3
    done

    "$SHELL_DIR/build.sh" "${build_args[@]}"
}

# Внутренний helper: проверяет integration-конфигурацию и наличие тестов.
integration_test_build_is_ready() {
    local ctest_output
    local cmake_cache_file="$BUILD_DIR/CMakeCache.txt"

    cmake_cache_matches \
        "$cmake_cache_file" \
        "${INTEGRATION_TEST_CMAKE_CONFIG[@]}" &&
        ctest_output="$(ctest --test-dir "$BUILD_DIR" -N -L integration 2>/dev/null)" &&
        grep -q "Total Tests: [1-9]" <<< "$ctest_output"
}

ensure_test_build() {
    if integration_test_build_is_ready; then
        log_info "Integration test artifacts found in: $BUILD_DIR" "$LOG_INDENT"
        return
    fi

    log_warn "Integration test build is missing or has incompatible configuration" "$LOG_INDENT"
    build_with_cmake_config "${INTEGRATION_TEST_CMAKE_CONFIG[@]}"

    if ! integration_test_build_is_ready; then
        log_error "Integration test artifacts were not produced in: $BUILD_DIR" "$LOG_INDENT"
        return 1
    fi

    log_ok "Integration test artifacts are ready" "$LOG_INDENT"
}

run_native() {
    local ctest_command=(
        ctest
        --test-dir "$BUILD_DIR"
        --progress
        --output-on-failure
        -L integration
        "$@"
    )
    local command_text

    log_stage "Integration test (native)"
    "$SHELL_DIR/build_ups_emulator.sh"
    ensure_test_build

    log_info "Running integration tests" "$LOG_INDENT"
    printf -v command_text '%q ' "${ctest_command[@]}"
    log_debug "Running: $command_text" "$LOG_SUBINDENT"
    "${ctest_command[@]}"
}

main() {
    if is_inside_docker; then
        run_native "$@"
        return
    fi

    log_stage "Integration test (Docker)"
    log_info "Running integration tests inside container" "$LOG_INDENT"
    docker_run ./scripts/itest.sh "$@"
}

main "$@"
