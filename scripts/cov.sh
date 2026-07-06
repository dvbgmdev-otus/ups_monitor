#!/usr/bin/env bash
#
# cov.sh — запуск покрытия с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → проверяет coverage-артефакты и запускает их нативно
#   - Если скрипт выполняется на хосте → запускает выполнение внутри Docker контейнера
#
# Поведение:
#   - Если coverage-скрипт отсутствует — запускает scripts/build.sh с нужными флагами
#   - После успешной сборки запускает coverage-скрипт
#   - Аргументы cov.sh пробрасываются в coverage-скрипт
#
# Коды возврата:
#   0 — покрытие успешно собрано
#   1 — ошибка сборки/запуска

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/docker.sh"

COVERAGE_SCRIPT_PATH="$TEST_BIN_DIR/run_coverage.sh"

ensure_coverage_build() {
    if [[ -x "$COVERAGE_SCRIPT_PATH" ]]; then
        log_info "Coverage script found: $COVERAGE_SCRIPT_PATH" "$LOG_INDENT"
        return 0
    fi

    log_warn "Coverage script not found. Starting build" "$LOG_INDENT"
    "$SHELL_DIR/build.sh" -DBUILD_TESTING=ON -DBUILD_COVERAGE=ON

    if [[ ! -x "$COVERAGE_SCRIPT_PATH" ]]; then
        log_error "Coverage script was not produced: $COVERAGE_SCRIPT_PATH" "$LOG_INDENT"
        return 1
    fi

    log_ok "Coverage artifacts are ready" "$LOG_INDENT"
}

run_native() {
    log_stage "Coverage (native)"
    ensure_coverage_build || return 1

    log_info "Running coverage script" "$LOG_INDENT"
    log_debug "Running: $COVERAGE_SCRIPT_PATH $*" "$LOG_SUBINDENT"
    "$COVERAGE_SCRIPT_PATH" "$@"
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
