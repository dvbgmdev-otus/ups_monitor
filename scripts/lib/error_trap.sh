#!/usr/bin/env bash
#
# error_trap.sh — библиотека обработки ошибок для bash (подключается через source).
#
# Назначение:
#   Настраивает единый trap для вывода места падения shell-скрипта.
#
# Пример использования:
#   set -eEuo pipefail
#   SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
#   # shellcheck disable=SC1091
#   source "$SCRIPT_DIR/lib/error_trap.sh"
#   setup_error_trap
#
# Автор: BGM

# Предотвращаем выполнение этого файла напрямую
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf "This script is a library and should not be executed directly\n" >&2
    exit 1
fi

# Защита от повторного включения
if [[ -z "${__ERROR_TRAP_SH_INCLUDED:-}" ]]; then
    readonly __ERROR_TRAP_SH_INCLUDED=1

    # Настраивает единый обработчик ошибок для shell-скриптов проекта
    setup_error_trap() {
        trap \
            'printf "\033[0;101m[ERROR] %s:%d: \"%s\" failed\033[0m\n" \
                "${BASH_SOURCE[0]:-${0}}" \
                "${LINENO}" \
                "${BASH_COMMAND}" \
                >&2' \
            ERR
    }
fi
