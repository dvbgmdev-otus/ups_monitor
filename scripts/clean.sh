#!/usr/bin/env bash
#
# clean.sh — очистка артефактов сборки проекта
#
# Описание:
#   Удаляет директорию сборки (BUILD_DIR).
#   Использует конфигурацию из config.sh.
#
# Поведение:
#   - Если директория сборки существует — удаляет её
#   - Если нет — выводит предупреждение
#
# Переменные окружения:
#   BUILD_DIR — директория сборки (по умолчанию задаётся в config.sh)
#
# Автор: BGM

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

# Главная функция
main() {
    log_stage "Clean"

    if [[ -d "$BUILD_DIR" ]]; then
        log_info "Removing build directory: $BUILD_DIR" "$LOG_INDENT"
        rm -rf "$BUILD_DIR"
        log_ok "Build directory removed" "$LOG_INDENT"
    else
        log_warn "Build directory not found: $BUILD_DIR" "$LOG_INDENT"
    fi
}

main "$@"
