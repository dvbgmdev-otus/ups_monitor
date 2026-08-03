#!/usr/bin/env bash
#
# test.sh — запуск всех тестов с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → подготавливает эмулятор,
#     проверяет полную тестовую конфигурацию и запускает тесты
#   - Если скрипт выполняется на хосте → запускает выполнение внутри Docker контейнера
#
# Поведение:
#   - Перед тестами собирает runtime UPS-эмулятора
#   - Если тестовая сборка отсутствует или имеет другую конфигурацию —
#     запускает scripts/build.sh
#   - После успешной сборки запускает все тесты через ctest
#   - Аргументы test.sh пробрасываются в ctest
#
# Коды возврата:
#   0 — все тесты успешно пройдены
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

ALL_TEST_CMAKE_CONFIG=(
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

# Внутренний helper: проверяет полную тестовую конфигурацию и наличие тестов.
all_test_build_is_ready() {
    local ctest_output
    local integration_test_output
    local cmake_cache_file="$BUILD_DIR/CMakeCache.txt"

    cmake_cache_matches \
        "$cmake_cache_file" \
        "${ALL_TEST_CMAKE_CONFIG[@]}" &&
        ctest_output="$(ctest --test-dir "$BUILD_DIR" -N 2>/dev/null)" &&
        grep -q "Total Tests: [1-9]" <<< "$ctest_output" &&
        integration_test_output="$(
            ctest --test-dir "$BUILD_DIR" -N -L integration 2>/dev/null
        )" &&
        grep -q "Total Tests: [1-9]" <<< "$integration_test_output"
}

ensure_test_build() {
    if all_test_build_is_ready; then
        log_info "All test artifacts found in: $BUILD_DIR" "$LOG_INDENT"
        return
    fi

    log_warn "Test build is missing or has incompatible configuration" "$LOG_INDENT"
    build_with_cmake_config "${ALL_TEST_CMAKE_CONFIG[@]}"

    if ! all_test_build_is_ready; then
        log_error "Test artifacts were not produced in: $BUILD_DIR" "$LOG_INDENT"
        return 1
    fi

    log_ok "All test artifacts are ready" "$LOG_INDENT"
}

run_native() {
    local ctest_command=(
        ctest
        --test-dir "$BUILD_DIR"
        --progress
        --output-on-failure
        "$@"
    )
    local command_text

    log_stage "Test (native)"
    "$SHELL_DIR/build_ups_emulator.sh"
    ensure_test_build

    log_info "Running all tests" "$LOG_INDENT"
    printf -v command_text '%q ' "${ctest_command[@]}"
    log_debug "Running: $command_text" "$LOG_SUBINDENT"
    "${ctest_command[@]}"
}

main() {
    if is_inside_docker; then
        run_native "$@"
        return
    fi

    log_stage "Test (Docker)"
    log_info "Running all tests inside container" "$LOG_INDENT"
    docker_run ./scripts/test.sh "$@"
}

main "$@"
