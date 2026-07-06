#!/usr/bin/env bash
#
# build.sh — сборка проекта (локально или в Docker)
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → выполняет нативную сборку (cmake)
#   - Если скрипт выполняется на хосте → запускает сборку внутри Docker контейнера
#
# Переменные окружения:
#   BUILD_DIR — директория сборки (по умолчанию: build)
#
# Коды возврата:
#   0 — успешная сборка
#   1 — ошибка сборки или инфраструктуры
#
# Автор: BGM

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/docker.sh"

# Запуск cmake
run_cmake() {
    local cmd_str
    printf -v cmd_str '%q ' "$@"
    log_debug "Running: ${cmd_str}" "$LOG_SUBINDENT"
    "$@" # вызов cmake
}

# Чистая сборка
build_native() {
    log_stage "Build (native)"

    log_info "Configuring project" "$LOG_INDENT"
    run_cmake cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" "$@"

    log_info "Building project" "$LOG_INDENT"
    run_cmake cmake --build "$BUILD_DIR"

    log_ok "Build completed" "$LOG_INDENT"
}

# Главная функция
main() {

    if is_inside_docker; then
        build_native "$@"
        return
    fi

    log_stage "Build (Docker)"
    log_info "Running build inside container" "$LOG_INDENT"

    docker_run ./scripts/build.sh "$@"
}

main "$@"
