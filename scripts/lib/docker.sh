#!/usr/bin/env bash
#
# docker.sh — общая инфраструктура для скриптов проекта
#
# Назначение:
#   - централизует конфигурацию (IMAGE_NAME, BUILD_DIR)
#   - предоставляет обёртки для работы с Docker
#   - инкапсулирует запуск контейнера (docker_run)
#
# Архитектура:
#   docker.sh          — orchestration layer
#   ensure_docker.sh   — проверка Docker runtime
#   ensure_image.sh    — проверка/сборка Docker image
#
# Использование:
#   source scripts/lib/docker.sh
#   docker_run <command>
# Автор: BGM
#

# Предотвращаем выполнение этого файла напрямую
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf "This script is a library and should not be executed directly\n" >&2
    exit 1
fi

# Защита от повторного включения
if [[ -z "${__DOCKER_SH_INCLUDED:-}" ]]; then
    readonly __DOCKER_SH_INCLUDED=1

    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
    # shellcheck disable=SC1091
    source "$SCRIPT_DIR/config.sh"
    # shellcheck disable=SC1091
    source "$LIB_DIR/logging.sh"

    # Проверка где выполняется скрипт (внутри контейнера или на хосте)
    is_inside_docker() {
        [[ -f /.dockerenv ]]
    }

    # Проверяет доступность Docker окружения, при необходимости пытается его запустить (только на macOS)
    ensure_docker() {
        "$SHELL_DIR/ensure_docker.sh"
    }

    # Проверяет наличие Docker образа, при необходимости собирает его
    ensure_image() {
        "$SHELL_DIR/ensure_image.sh"
    }

    # Запускает команду внутри Docker контейнера
    docker_run() {
        ensure_docker
        ensure_image

        log_info "Running container: $IMAGE_NAME" "$LOG_INDENT"

        local docker_args=(--rm --init)

        # environment
        docker_args+=(
            -e LOG_LEVEL="${LOG_LEVEL:-info}"
            -e CLANG_TIDY_MODE="${CLANG_TIDY_MODE:-light}"
        )

        # TTY
        if [[ -t 1 && -t 0 ]]; then
            docker_args+=(-it)
        fi

        # Linux UID mapping
        if [[ "$(uname -s)" == "Linux" ]]; then
            docker_args+=(-u "$(id -u):$(id -g)")
        fi

        # volumes
        docker_args+=(
            -v "$PROJECT_ROOT":/app
            -w /app
        )

        local cmd_str
        printf -v cmd_str "%q " "$@"
        log_debug "Command: docker run ${docker_args[*]} $IMAGE_NAME $cmd_str" "$LOG_SUBINDENT"

        docker run \
            "${docker_args[@]}" \
            "$IMAGE_NAME" \
            "$@"
    }
fi
