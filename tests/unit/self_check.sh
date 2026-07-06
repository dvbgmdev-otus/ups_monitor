#!/usr/bin/env bash
set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/../../scripts/lib/config.sh"
# shellcheck disable=SC1091
source "$LIB_DIR/logging.sh"

BINARY="${1:-$BIN_DIR/ups_monitor}"

check_output() {
    local name="$1"
    local expected="$2"
    local actual="$3"

    if [[ "$actual" != "$expected" ]]; then
        log_error "Test failed: $name"
        log_error "Expected: [$expected]"
        log_error "Actual:   [$actual]"
        exit 1
    fi

    log_ok "Passed: $name"
}

main() {
    if [[ ! -x "$BINARY" ]]; then
        log_error "Binary not found or not executable: $BINARY"
        exit 1
    fi

    local output
    output="$("$BINARY")"

    local expected
    expected="Application started"

    check_output "full program output" "$expected" "$output"

    log_ok "Self-check passed"
}

main "$@"