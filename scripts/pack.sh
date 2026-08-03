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
source "$LIB_DIR/build_artifacts.sh"

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
