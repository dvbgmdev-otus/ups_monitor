#!/usr/bin/env bash
#
# ensure_docker.sh — проверка доступности Docker окружения
#
# Контракт:
#   - Если Docker доступен → завершает выполнение с кодом 0
#   - Если Docker недоступен:
#       • на macOS — пытается запустить Colima (если не указан --check)
#       • на Linux — выводит инструкцию по запуску и завершает работу с ошибкой
#
# Автор: BGM

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

CHECK_ONLY=false

# Функция для проверки аргументов
parse_args() {
    while [[ "$#" -gt 0 ]]; do
        case "$1" in
            --check)
                CHECK_ONLY=true
                shift
                ;;
            *)
                log_error "Unknown argument: $1" "$LOG_INDENT"
                log_text "Usage: $0 [--check]" "$LOG_SUBINDENT"
                exit 1
                ;;
        esac
    done
}

# Функция для проверки наличия Docker CLI
ensure_docker_cli() {
    if ! command -v docker >/dev/null 2>&1; then
        log_error "docker CLI not installed" "$LOG_INDENT"
        exit 1
    fi
}

# Функция для проверки, запущен ли Docker daemon
docker_daemon_running() {
    docker info >/dev/null 2>&1
}

# Функция для проверки Docker окружения
check_docker_environment() {
    local os
    os="$(uname -s)"
    case "$os" in

        Darwin)
            log_info "Detected OS: macOS" "$LOG_INDENT"
            if $CHECK_ONLY; then
                log_error "Docker is not accessible." "$LOG_INDENT"
                log_text "Run:" "$LOG_SUBINDENT"
                log_text "colima start" "$LOG_SUBINDENT"
                exit 1
            fi

            if command -v colima >/dev/null 2>&1; then
                log_info "Starting Colima..." "$LOG_INDENT"
                if ! colima start; then
                    log_error "Failed to start Colima" "$LOG_INDENT"
                    log_text "Check log:" "$LOG_SUBINDENT"
                    log_text "$HOME/.colima/_lima/colima/ha.stderr.log" "$LOG_SUBINDENT"
                    log_text "Recovery:" "$LOG_SUBINDENT"
                    log_text "scripts/reset_colima.sh" "$LOG_SUBINDENT"
                    exit 1
                fi
                wait_for_docker
            else
                log_error "Colima not installed" "$LOG_INDENT"
                log_text "Install:" "$LOG_SUBINDENT"
                log_text "brew install colima" "$LOG_SUBINDENT"
                exit 1
            fi
            ;;

        Linux)
            log_info "Detected OS: Linux" "$LOG_INDENT"
            log_error "Docker is not accessible." "$LOG_INDENT"
            log_text "Run:" "$LOG_SUBINDENT"
            log_text "sudo systemctl start docker" "$LOG_SUBINDENT"
            exit 1
            ;;

        *)
            log_error "Unsupported OS: $os" "$LOG_INDENT"
            exit 1
            ;;
    esac
}

# Функция для ожидания запуска Docker daemon
wait_for_docker() {
    log_info "Waiting for Docker daemon..." "$LOG_INDENT"
    local max_wait=30
    for ((i=0; i<max_wait; i++)); do
        if docker_daemon_running; then
            return
        fi
        sleep 1
    done
    log_error "Docker daemon did not start within ${max_wait}s" "$LOG_INDENT"
    exit 1
}

# Главная функция
main() {
    log_stage "Docker environment check"
    parse_args "$@"
    ensure_docker_cli
    if docker_daemon_running; then
        log_ok "Docker already running" "$LOG_INDENT"
        exit 0
    fi
    # если дошли до сюда, то Docker не запущен 
    check_docker_environment
}

main "$@"
