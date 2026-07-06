#!/usr/bin/env bash
#
# cppcheck.sh — статический анализ кода с помощью Cppcheck
#
# Автор: BGM    

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

CPPCHECK_FLAGS=(
    --std=c++11                     # Использовать стандарт C++11
    --enable=warning                # Реальные проблемы
    --enable=performance            # Потенциальные узкие места
    --enable=portability            # Переносимость
    # --enable=style                  # Стиль — по необходимости
    --suppress=missingIncludeSystem # Подавление предупреждений о недоступных системных include
    --check-level=exhaustive        # Максимальный уровень глубины проверок
    --inconclusive                  # Включение предположительных (inconclusive) проверок
    --inline-suppr                  # Разрешить inline-подавления (// cppcheck-suppress)
    --error-exitcode=2              # Выход с кодом 2 при обнаружении ошибок
    # --quiet                         # Минимизировать вывод (только предупреждения и ошибки)
    -I"$SRC_DIR"                     # Добавить src в include-path
)

# Если аргументы не переданы — анализируем src
TARGETS=("$@")
if [[ ${#TARGETS[@]} -eq 0 ]]; then
    TARGETS=("$SRC_DIR")
fi

log_flags() {
    local flags_str=""
    local spaces
    printf -v spaces '%*s' "${LOG_SUBINDENT}" ''
    for flag in "${CPPCHECK_FLAGS[@]}"; do
        printf -v flags_str '%s\n%s%s' "$flags_str" "$spaces" "$flag"
    done
    log_debug "Cppcheck Flags:$flags_str" "$LOG_INDENT"
}

main() {
    log_stage "Cppcheck Static Analysis"
    log_flags # Выводим флаги для отладки (только при LOG_LEVEL=debug)

    cppcheck "${CPPCHECK_FLAGS[@]}" "${TARGETS[@]}"

    log_ok "Cppcheck analysis completed successfully" "$LOG_INDENT"
}

main "$@"
