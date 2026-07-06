#!/usr/bin/env bash
#
# pack.sh — сборка DEB-пакета с автосборкой при необходимости
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → проверяет бинарник и запускает package target
#   - Если скрипт выполняется на хосте → запускает сборку пакета внутри Docker контейнера
#
# Поведение:
#   - Если бинарник отсутствует или не исполняемый — запускает scripts/build.sh
#   - После успешной сборки запускает CPack через target package
#   - Аргументы pack.sh пробрасываются в cmake --build
#
# Коды возврата:
#   0 — пакет успешно собран
#   1 — ошибка сборки/пакетирования

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
PACK_BIN_PATH=""

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

    readonly PACK_BIN_PATH="$BIN_DIR/$project_name"
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

    if [[ -x "$PACK_BIN_PATH" ]]; then
        log_info "Binary found: $PACK_BIN_PATH" "$LOG_INDENT"
        return 0
    fi

    log_warn "Binary not found. Starting build" "$LOG_INDENT"
    "$SHELL_DIR/build.sh"

    if [[ ! -x "$PACK_BIN_PATH" ]]; then
        log_error "Binary was not produced: $PACK_BIN_PATH" "$LOG_INDENT"
        return 1
    fi

    log_ok "Binary is ready: $PACK_BIN_PATH" "$LOG_INDENT"
}

run_native() {
    log_stage "Pack (native)"
    ensure_build

    log_info "Building package" "$LOG_INDENT"
    log_debug "Running: cmake --build \"$BUILD_DIR\" --target package $*" "$LOG_SUBINDENT"
    cmake --build "$BUILD_DIR" --target package "$@"

    log_ok "Package build completed" "$LOG_INDENT"
}

main() {
    if is_inside_docker; then
        run_native "$@"
        return
    fi

    log_stage "Pack (Docker)"
    log_info "Building package inside container" "$LOG_INDENT"
    docker_run ./scripts/pack.sh "$@"
}

main "$@"
