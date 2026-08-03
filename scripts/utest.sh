#!/usr/bin/env bash
#
# utest.sh — запуск unit-тестов с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → проверяет unit-конфигурацию и запускает тесты
#   - Если скрипт выполняется на хосте → запускает выполнение внутри Docker контейнера
#
# Поведение:
#   - Если unit-сборка отсутствует или имеет другую конфигурацию — запускает scripts/build.sh
#   - После успешной сборки запускает тесты через ctest
#   - Аргументы utest.sh пробрасываются в ctest
#
# Коды возврата:
#   0 — все unit-тесты успешно пройдены
#   1 — ошибка сборки/запуска тестов

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

UNIT_TEST_CMAKE_CONFIG=(
    BUILD_TESTING BOOL ON
    BUILD_COVERAGE BOOL OFF
    BUILD_INTEGRATION_TESTS BOOL OFF
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

# Внутренний helper: проверяет конфигурацию unit-сборки и наличие тестов.
unit_test_build_is_ready() {
    local ctest_output
    local cmake_cache_file="$BUILD_DIR/CMakeCache.txt"

    cmake_cache_matches \
        "$cmake_cache_file" \
        "${UNIT_TEST_CMAKE_CONFIG[@]}" &&
        ctest_output="$(ctest --test-dir "$BUILD_DIR" -N 2>/dev/null)" &&
        grep -q "Total Tests: [1-9]" <<< "$ctest_output"
}

ensure_test_build() {
    if unit_test_build_is_ready; then
        log_info "Unit test artifacts found in: $BUILD_DIR" "$LOG_INDENT"
        return
    fi

    log_warn "Unit test build is missing or has incompatible configuration" "$LOG_INDENT"
    build_with_cmake_config "${UNIT_TEST_CMAKE_CONFIG[@]}"

    if ! unit_test_build_is_ready; then
        log_error "Unit test artifacts were not produced in: $BUILD_DIR" "$LOG_INDENT"
        return 1
    fi

    log_ok "Unit test artifacts are ready" "$LOG_INDENT"
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

    log_stage "Unit test (native)"
    ensure_test_build

    log_info "Running unit tests" "$LOG_INDENT"
    printf -v command_text '%q ' "${ctest_command[@]}"
    log_debug "Running: $command_text" "$LOG_SUBINDENT"
    "${ctest_command[@]}"
}

main() {
    if is_inside_docker; then
        run_native "$@"
        return
    fi

    log_stage "Unit test (Docker)"
    log_info "Running unit tests inside container" "$LOG_INDENT"
    docker_run ./scripts/utest.sh "$@"
}

main "$@"
