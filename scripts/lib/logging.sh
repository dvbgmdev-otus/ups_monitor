#!/usr/bin/env bash
#
# logging.sh — библиотека логирования для bash (подключается через source).
#
# Возможности:
#   - уровни: debug, info, warn, error
#   - фильтрация сообщений через LOG_LEVEL
#   - динамическое изменение уровня (set_log_level)
#   - цветной вывод (при поддержке терминала)
#   - поддержка отступов
#   - вспомогательные функции: stage, ok, separator
#
# Пример использования:
#   SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
#   # shellcheck disable=SC1091
#   source "$SCRIPT_DIR/logging.sh"
#   LOG_LEVEL=info
#   log_stage "Запуск скрипта"
#   log_warn "Предупреждение" 3
#   print_separator 3 50
#
# Автор: BGM

# Предотвращаем выполнение этого файла напрямую
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf "This script is a library and should not be executed directly\n" >&2
    exit 1
fi

# Защита от повторного включения
if [[ -z "${__LOGGING_SH_INCLUDED:-}" ]]; then
    readonly __LOGGING_SH_INCLUDED=1

    #region ====================== НАСТРОЙКА ЦВЕТОВ ======================
    USE_COLOR_STDOUT=0
    USE_COLOR_STDERR=0
    [[ -t 1 ]] && USE_COLOR_STDOUT=1
    [[ -t 2 ]] && USE_COLOR_STDERR=1

    RED='\033[0;31m'    # ${RED}
    GREEN='\033[0;32m'  # ${GREEN}
    YELLOW='\033[0;93m' # ${YELLOW}
    CYAN='\033[0;36m'   # ${CYAN}
    BLUE='\033[0;34m'   # ${BLUE}
    BOLD='\033[1m'      # ${BOLD}
    NC='\033[0m'        # ${NC}
    #endregion

    #region ====================== НАСТРОЙКА ЛОГИРОВАНИЯ ======================
    LOG_LEVEL=${LOG_LEVEL:-info}

    # Установка уровня логирования
    set_log_level() {
        case "${1:-}" in
            debug|info|warn|error)
                LOG_LEVEL="$1"
                ;;
            *)
                printf "[ERROR] Invalid log level: %s\n" "${1:-<empty>}" >&2
                return 1
                ;;
        esac
    }

    # возможные значения: debug, info, warn, error
    level_to_num() {
        case "$1" in
            debug) printf 0 ;;
            info)  printf 1 ;;
            warn)  printf 2 ;;
            error) printf 3 ;;
            *)     printf 1 ;; # default: info
        esac
    }

    # Функция для проверки, нужно ли логировать сообщение данного уровня
    should_log() {
        local msg_level
        local log_level
        msg_level=$(level_to_num "$1")
        log_level=$(level_to_num "$LOG_LEVEL")
        [[ ${msg_level} -ge ${log_level} ]] # логируем, если уровень сообщения выше или равен текущему уровню
    }
    #endregion

    #region ====================== HELPER ФУНКЦИИ ======================
    # Внутренняя функция для логирования с цветами и отступами
    _log() {
        local level="$1"
        local message="${2:-}"
        local indent="${3:-0}"
        local prefix="${4:-}"
        local color="${5:-}"

        if ! should_log "${level}"; then
            return
        fi

        local fd=1
        case "${level}" in
            warn|error) fd=2 ;; # stderr
            *)          fd=1 ;; # stdout
        esac

        local spaces
        local text
        printf -v spaces '%*s' "${indent}" ''

        # формируем текст с отступом и префиксом
        if [[ -n "${prefix}" ]]; then
            text="${spaces}${prefix} ${message}"
        else
            text="${spaces}${message}"
        fi

        if [[ "$fd" -eq 1 && "$USE_COLOR_STDOUT" -eq 1 ]] || \
        [[ "$fd" -eq 2 && "$USE_COLOR_STDERR" -eq 1 ]]; then
            printf "%b%s%b\n" "${color}" "${text}" "${NC}" >&"$fd"
        else
            printf "%s\n" "${text}" >&"$fd"
        fi
    }
    #endregion

    #region ====================== API ДЛЯ ЛОГИРОВАНИЯ ======================
    # Уровни логирования
    log_debug() {
        _log debug "$1" "${2:-}" "[DEBUG]" "$YELLOW"
    }

    log_info() {
        _log info "$1" "${2:-}" "[INFO]" "$CYAN"
    }

    log_warn() {
        _log warn "$1" "${2:-}" "[WARN]" "$YELLOW"
    }

    log_error() {
        _log error "$1" "${2:-}" "[ERROR]" "$RED"
    }

    # Специальные функции
    log_stage() {
        _log info "$1" "${2:-}" "===>" "$BLUE$BOLD"
    }

    log_ok() {
        _log info "$1" "${2:-}" "[OK]" "$GREEN"
    }

    log_text() {
        _log info "$1" "${2:-}"
    }

    print_separator() {
        if ! should_log info; then
            return
        fi
        local default_total_width=70 # ширина по умолчанию для разделителя
        local default_separator_char='-' # символ для разделителя
        local indent="${1:-0}"
        local total_width="${2:-${default_total_width}}"
        local separator_char="${3:-${default_separator_char}}"
        separator_char="${separator_char:0:1}" # берем только первый символ

        local content_width=$((total_width - indent))
        (( content_width < 1 )) && content_width=1

        local indent_str
        local line
        printf -v indent_str '%*s' "${indent}" ""
        printf -v line '%*s' "${content_width}" ""

        line=${line// /${separator_char}}

        printf "%s%s\n" "${indent_str}" "${line}"
    }
    #endregion
fi
