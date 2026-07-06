#!/usr/bin/env bash
#
# reset_colima.sh — аварийное восстановление Colima на macOS
#
# Описание:
#   Очищает зависшие процессы Colima/Lima, останавливает Colima
#   и пытается запустить её заново.
#
# Контракт:
#   - Если Colima успешно восстановлена → завершает выполнение с кодом 0
#   - Если восстановить не удалось → завершает выполнение с кодом 1
#
# Автор: BGM

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

docker_daemon_running() {
    docker info >/dev/null 2>&1
}

ensure_macos() {
    local os
    os="$(uname -s)"

    if [[ "$os" != "Darwin" ]]; then
        log_error "reset_colima.sh is supported only on macOS" "$LOG_INDENT"
        exit 1
    fi
}

ensure_colima_installed() {
    if ! command -v colima >/dev/null 2>&1; then
        log_error "Colima not installed" "$LOG_INDENT"
        log_text "Install:" "$LOG_SUBINDENT"
        log_text "brew install colima" "$LOG_SUBINDENT"
        exit 1
    fi
}

cleanup_processes() {
    log_info "Stopping stale Colima/Lima processes" "$LOG_INDENT"

    pkill -f '/colima' || true
    pkill -f 'limactl' || true
    pkill -f 'hostagent' || true
}

stop_colima() {
    log_info "Stopping Colima" "$LOG_INDENT"
    colima stop -f >/dev/null 2>&1 || true
}

start_colima() {
    log_info "Starting Colima" "$LOG_INDENT"
    if ! colima start; then
        log_error "Failed to start Colima" "$LOG_INDENT"
        log_text "Check log:" "$LOG_SUBINDENT"
        log_text "$HOME/.colima/_lima/colima/ha.stderr.log" "$LOG_SUBINDENT"
        exit 1
    fi
}

wait_for_docker() {
    log_info "Waiting for Docker daemon..." "$LOG_INDENT"

    local max_wait=30
    local i

    for ((i=0; i<max_wait; i++)); do
        if docker_daemon_running; then
            log_ok "Docker is available" "$LOG_INDENT"
            return
        fi
        sleep 1
    done

    log_error "Docker daemon did not become available within ${max_wait}s" "$LOG_INDENT"
    exit 1
}

main() {
    log_stage "Reset Colima"

    ensure_macos
    ensure_colima_installed

    cleanup_processes
    stop_colima
    start_colima
    wait_for_docker
}

main "$@"
