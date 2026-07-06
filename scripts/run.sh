#!/usr/bin/env bash
#
# run.sh — запуск приложения с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → проверяет бинарник и запускает его нативно
#   - Если скрипт выполняется на хосте → запускает выполнение внутри Docker контейнера
#
# Поведение:
#   - Если бинарник отсутствует или не исполняемый — запускает scripts/build.sh
#   - После успешной сборки запускает бинарник
#   - Аргументы run.sh пробрасываются в бинарник
#
# Коды возврата:
#   0 — успешный запуск
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

# Глобальная переменная для пути к бинарнику
RUN_BIN_PATH=""

get_binary_path() {
    local project_name

    if [[ ! -f "$PROJECT_BINARY_NAME_FILE" ]]; then
        log_error "Binary name file not found: $PROJECT_BINARY_NAME_FILE" "$LOG_INDENT"
        return 1
    fi

    project_name="$(tr -d '\n' < "$PROJECT_BINARY_NAME_FILE")"

    if [[ -z "$project_name" ]]; then
        log_error "Binary name file is empty: $PROJECT_BINARY_NAME_FILE" "$LOG_INDENT"
        return 1
    fi

    readonly RUN_BIN_PATH="$BIN_DIR/$project_name"
}

ensure_binary_name_file() {
    if [[ -f "$PROJECT_BINARY_NAME_FILE" ]]; then
        return 0
    fi

    log_warn "Binary name file not found. Starting build" "$LOG_INDENT"
    "$SHELL_DIR/build.sh"

    if [[ ! -f "$PROJECT_BINARY_NAME_FILE" ]]; then
        log_error "Binary name file was not produced: $PROJECT_BINARY_NAME_FILE" "$LOG_INDENT"
        return 1
    fi
}

ensure_build() {
    ensure_binary_name_file || return 1
    get_binary_path || return 1

    if [[ -x "$RUN_BIN_PATH" ]]; then
        log_info "Binary found: $RUN_BIN_PATH" "$LOG_INDENT"
        return 0
    fi

    log_warn "Binary not found. Starting build" "$LOG_INDENT"
    "$SHELL_DIR/build.sh"

    if [[ ! -x "$RUN_BIN_PATH" ]]; then
        log_error "Binary was not produced: $RUN_BIN_PATH" "$LOG_INDENT"
        return 1
    fi

    log_ok "Binary is ready: $RUN_BIN_PATH" "$LOG_INDENT"
}

run_native() {
    log_stage "Run (native)"
    ensure_build || return 1

    log_info "Starting binary" "$LOG_INDENT"
    log_debug "Running: $RUN_BIN_PATH $*" "$LOG_SUBINDENT"
    "$RUN_BIN_PATH" "$@"
}

main() {
    if is_inside_docker; then
        run_native "$@"
        return
    fi

    log_stage "Run (Docker)"
    log_info "Running application inside container" "$LOG_INDENT"
    docker_run ./scripts/run.sh "$@"
}

main "$@"
