#!/usr/bin/env bash
#
# ensure_image.sh — проверка и (при необходимости) сборка Docker образа
#
# Контракт:
#   - Проверяет доступность Docker daemon
#   - Проверяет наличие Docker образа IMAGE_NAME
#   - Если образ отсутствует:
#       • при AUTO_BUILD_IMAGE=1 — собирает образ
#       • иначе — завершает выполнение с ошибкой
#
# Переменные окружения:
#   IMAGE_NAME        — имя Docker образа (по умолчанию: otus_builder)
#   DOCKERFILE_DIR    — директория с Dockerfile (по умолчанию: ../docker)
#   AUTO_BUILD_IMAGE  — авто-сборка образа (1 — включено, 0 — отключено)
#
# Коды возврата:
#   0 — успех (образ существует или успешно собран)
#   1 — ошибка (Docker недоступен, отсутствует Dockerfile, ошибка сборки и т.п.)
#
# Автор: BGM

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

# Проверка наличия Docker файла
DOCKERFILE_DIR="${DOCKERFILE_DIR:-$SCRIPT_DIR/../docker}"
DOCKERFILE_PATH="$DOCKERFILE_DIR/Dockerfile"
if [[ ! -f "$DOCKERFILE_PATH" ]]; then
    log_error "Dockerfile not found: $DOCKERFILE_PATH" "$LOG_INDENT"
    exit 1
fi

# Проверка запущен ли Docker daemon
docker_daemon_running() {
    docker info >/dev/null 2>&1
}

# Проверка наличия Docker образа
docker_image_exists() {
    docker image inspect "$IMAGE_NAME" >/dev/null 2>&1
}

# Сборка Docker образа
build_image() {
    log_stage "Building Docker image"
    log_info "Context: $DOCKERFILE_DIR" "$LOG_INDENT"

    if ! docker build -t "$IMAGE_NAME" "$DOCKERFILE_DIR"; then
        log_error "Failed to build image '$IMAGE_NAME'" "$LOG_INDENT"
        exit 1
    fi

    log_ok "Image '$IMAGE_NAME' built" "$LOG_INDENT"
}

# Главная функция
main() {
    log_stage "Ensure Docker image"

    if ! docker_daemon_running; then
        log_error "Docker daemon is not running" "$LOG_INDENT"
        exit 1
    fi

    if docker_image_exists; then
        log_ok "Image '$IMAGE_NAME' already exists" "$LOG_INDENT"
        exit 0
    fi

    log_warn "Image '$IMAGE_NAME' not found" "$LOG_INDENT"

    if [[ "${AUTO_BUILD_IMAGE:-1}" == "1" ]]; then
        build_image
    else
        log_text "Run manually:" "$LOG_SUBINDENT"
        log_text "$SCRIPT_DIR/ensure_image.sh" "$LOG_SUBINDENT"
        exit 1
    fi
}

main "$@"
