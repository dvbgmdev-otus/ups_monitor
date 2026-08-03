#!/usr/bin/env bash
#
# build_artifacts.sh — общие проверки build-артефактов проекта
#
# Использование:
#   source scripts/lib/build_artifacts.sh

# Предотвращаем выполнение этого файла напрямую
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf "This script is a library and should not be executed directly\n" >&2
    exit 1
fi

# Защита от повторного включения
if [[ -z "${__BUILD_ARTIFACTS_SH_INCLUDED:-}" ]]; then
    readonly __BUILD_ARTIFACTS_SH_INCLUDED=1

    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
    # shellcheck disable=SC1091
    source "$SCRIPT_DIR/config.sh"
    # shellcheck disable=SC1091
    source "$LIB_DIR/logging.sh"

    # Путь к основному бинарнику проекта после успешной проверки build-артефактов.
    PROJECT_BIN_PATH=""

    # Получает путь к основному бинарнику проекта по имени, сгенерированному CMake.
    get_binary_path() {
        local project_name

        if [[ ! -f "$PROJECT_BINARY_NAME_FILE" ]]; then
            log_error "Binary name file not found: $PROJECT_BINARY_NAME_FILE" "$LOG_INDENT"
            return 1
        fi

        project_name="$(tr -d '\n' < "$PROJECT_BINARY_NAME_FILE")"

        if [[ -z "$project_name" ]]; then
            log_error "Binary name file is empty: $PROJECT_BINARY_NAME_FILE" "$LOG_INDENT"
            return 1
        fi

        PROJECT_BIN_PATH="$BIN_DIR/$project_name"
    }

    # Проверяет наличие файла с именем основного бинарника.
    # Если файл отсутствует — запускает сборку проекта.
    ensure_binary_name_file() {
        if [[ -f "$PROJECT_BINARY_NAME_FILE" ]]; then
            return 0
        fi

        log_warn "Binary name file not found. Starting build" "$LOG_INDENT"
        "$SHELL_DIR/build.sh"

        if [[ ! -f "$PROJECT_BINARY_NAME_FILE" ]]; then
            log_error "Binary name file was not produced: $PROJECT_BINARY_NAME_FILE" "$LOG_INDENT"
            return 1
        fi
    }

    # Проверяет, что основной бинарник проекта собран и исполняемый.
    ensure_build() {
        ensure_binary_name_file || return 1
        get_binary_path || return 1

        if [[ -x "$PROJECT_BIN_PATH" ]]; then
            log_info "Binary found: $PROJECT_BIN_PATH" "$LOG_INDENT"
            return 0
        fi

        log_warn "Binary not found. Starting build" "$LOG_INDENT"
        "$SHELL_DIR/build.sh"

        if [[ ! -x "$PROJECT_BIN_PATH" ]]; then
            log_error "Binary was not produced: $PROJECT_BIN_PATH" "$LOG_INDENT"
            return 1
        fi

        log_ok "Binary is ready: $PROJECT_BIN_PATH" "$LOG_INDENT"
    }
fi
