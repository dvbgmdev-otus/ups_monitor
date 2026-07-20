#!/usr/bin/env bash
#
# cov.sh — запуск покрытия с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → проверяет coverage-артефакты и запускает их нативно
#   - Если скрипт выполняется на хосте → запускает выполнение внутри Docker контейнера
#
# Поведение:
#   - Если coverage-сборка отсутствует или имеет другую конфигурацию —
#     запускает scripts/build.sh
#   - После успешной сборки запускает coverage-скрипт
#   - Аргументы cov.sh пробрасываются в coverage-скрипт
#
# Коды возврата:
#   0 — покрытие успешно собрано
#   1 — ошибка сборки/запуска

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

COVERAGE_SCRIPT_PATH="$TEST_BIN_DIR/run_coverage.sh"

COVERAGE_CMAKE_CONFIG=(
    BUILD_TESTING BOOL ON
    BUILD_COVERAGE BOOL ON
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

# Внутренний helper: проверяет coverage-конфигурацию и наличие артефактов.
coverage_build_is_ready() {
    local ctest_output
    local cmake_cache_file="$BUILD_DIR/CMakeCache.txt"

    cmake_cache_matches \
        "$cmake_cache_file" \
        "${COVERAGE_CMAKE_CONFIG[@]}" &&
        [[ -x "$COVERAGE_SCRIPT_PATH" ]] &&
        ctest_output="$(ctest --test-dir "$BUILD_DIR" -N 2>/dev/null)" &&
        grep -q "Total Tests: [1-9]" <<< "$ctest_output"
}

ensure_coverage_build() {
    if coverage_build_is_ready; then
        log_info "Coverage artifacts found in: $BUILD_DIR" "$LOG_INDENT"
        return
    fi

    log_warn "Coverage build is missing or has incompatible configuration" "$LOG_INDENT"
    build_with_cmake_config "${COVERAGE_CMAKE_CONFIG[@]}"

    if ! coverage_build_is_ready; then
        log_error "Coverage artifacts were not produced in: $BUILD_DIR" "$LOG_INDENT"
        return 1
    fi

    log_ok "Coverage artifacts are ready" "$LOG_INDENT"
}

run_native() {
    local coverage_command=(
        env
        CTEST_PROGRESS_OUTPUT=1
        "$COVERAGE_SCRIPT_PATH"
        "$@"
    )
    local command_text

    log_stage "Coverage (native)"
    ensure_coverage_build

    log_info "Running coverage script" "$LOG_INDENT"
    printf -v command_text '%q ' "${coverage_command[@]}"
    log_debug "Running: $command_text" "$LOG_SUBINDENT"
    "${coverage_command[@]}"
}

open_report_on_host() {
    local report_path="${BUILD_DIR}/out/index.html"

    if [[ ! -f "$report_path" ]]; then
        log_warn "Coverage report not found: $report_path" "$LOG_INDENT"
        return 0
    fi

    log_info "Opening coverage report" "$LOG_INDENT"
    log_debug "Opening: $report_path" "$LOG_SUBINDENT"

    if [[ "$OSTYPE" == "darwin"* ]]; then
        open "$report_path" >/dev/null 2>&1 || true
    else
        xdg-open "$report_path" >/dev/null 2>&1 || true
    fi
}

main() {
    if is_inside_docker; then
        run_native "$@"
        return
    fi

    log_stage "Coverage (Docker)"
    log_info "Running coverage inside container" "$LOG_INDENT"
    docker_run ./scripts/cov.sh "$@"

    open_report_on_host
}

main "$@"
