#!/usr/bin/env bash
#
# build_ups_emulator.sh — сборка UPS emulator для интеграционных тестов
#
# Контракт:
#   - Если скрипт выполняется внутри Docker → собирает эмулятор нативно
#   - Если скрипт выполняется на хосте → запускает сборку внутри Docker контейнера
#   - После сборки копирует runtime-набор в build/bin/tests/ups_emulator
#
# Коды возврата:
#   0 — успешная сборка и подготовка runtime-артефактов
#   1 — ошибка сборки или инфраструктуры

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

UPS_EMULATOR_SRC_DIR="$PROJECT_ROOT/tests/ups_emulator"
UPS_EMULATOR_BUILD_DIR="$UPS_EMULATOR_SRC_DIR/build"
UPS_EMULATOR_RUNTIME_DIR="$TEST_BIN_DIR/ups_emulator"

run_command() {
    local cmd_str
    printf -v cmd_str '%q ' "$@"
    log_debug "Running: ${cmd_str}" "$LOG_SUBINDENT"
    "$@"
}

prepare_runtime() {
    local emulator_bin="$UPS_EMULATOR_BUILD_DIR/bin/ups_emulator"
    local emulator_config_dir="$UPS_EMULATOR_BUILD_DIR/bin/config"

    if [[ ! -x "$emulator_bin" ]]; then
        log_error "UPS emulator binary not found: $emulator_bin" "$LOG_INDENT"
        return 1
    fi

    if [[ ! -d "$emulator_config_dir" ]]; then
        log_error "UPS emulator config directory not found: $emulator_config_dir" "$LOG_INDENT"
        return 1
    fi

    log_info "Preparing runtime directory: $UPS_EMULATOR_RUNTIME_DIR" "$LOG_INDENT"
    mkdir -p "$UPS_EMULATOR_RUNTIME_DIR"
    cp "$emulator_bin" "$UPS_EMULATOR_RUNTIME_DIR/"
    rm -rf "$UPS_EMULATOR_RUNTIME_DIR/config"
    cp -R "$emulator_config_dir" "$UPS_EMULATOR_RUNTIME_DIR/"

    log_ok "UPS emulator runtime is ready: $UPS_EMULATOR_RUNTIME_DIR" "$LOG_INDENT"
}

build_native() {
    log_stage "UPS emulator build (native)"

    if [[ ! -d "$UPS_EMULATOR_SRC_DIR" ]]; then
        log_error "UPS emulator submodule not found: $UPS_EMULATOR_SRC_DIR" "$LOG_INDENT"
        log_info "Run: git submodule update --init --recursive" "$LOG_INDENT"
        return 1
    fi

    log_info "Configuring UPS emulator" "$LOG_INDENT"
    run_command cmake \
        -S "$UPS_EMULATOR_SRC_DIR" \
        -B "$UPS_EMULATOR_BUILD_DIR" \
        -DBUILD_TESTING=OFF \
        -DBUILD_COVERAGE=OFF \
        -DBUILD_GUI=OFF

    log_info "Building UPS emulator" "$LOG_INDENT"
    run_command cmake --build "$UPS_EMULATOR_BUILD_DIR"

    prepare_runtime
}

main() {
    if is_inside_docker; then
        build_native "$@"
        return
    fi

    log_stage "UPS emulator build (Docker)"
    log_info "Running UPS emulator build inside container" "$LOG_INDENT"

    docker_run ./scripts/build_ups_emulator.sh "$@"
}

main "$@"
