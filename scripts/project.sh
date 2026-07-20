#!/usr/bin/env bash
#
# project.sh — единая точка входа в инфраструктуру проекта
#
# Назначение:
#   Маршрутизирует пользовательские команды к специализированным скриптам.
#
# Использование:
#   ./scripts/project.sh build
#   ./scripts/project.sh utest
#   ./scripts/project.sh itest
#   ./scripts/project.sh run
#   ./scripts/project.sh pack
#   ./scripts/project.sh cov
#   ./scripts/project.sh build-emulator
#   ./scripts/project.sh cppcheck
#   ./scripts/project.sh tidy
#   ./scripts/project.sh clean
#   ./scripts/project.sh help
#
# Автор: BGM
#

set -eEuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
# shellcheck disable=SC1091
source "$SCRIPT_DIR/lib/error_trap.sh"
setup_error_trap

usage() {
cat <<'EOF'
Usage:
  ./scripts/project.sh <command> [args]

Commands:
  build           Build project
  utest           Run unit tests
  itest           Run integration tests
  run             Run application
  pack            Build DEB package
  cov             Run coverage report
  build-emulator  Build UPS emulator test runtime
  clean           Remove build artifacts
  cppcheck        Run static analysis with Cppcheck
  tidy            Run static analysis with clang-tidy
  help            Show this help

Examples:
  ./scripts/project.sh build
  ./scripts/project.sh utest
  ./scripts/project.sh itest
  ./scripts/project.sh run
  ./scripts/project.sh pack
  ./scripts/project.sh cov
  ./scripts/project.sh build-emulator
  ./scripts/project.sh cppcheck
  ./scripts/project.sh tidy
  ./scripts/project.sh clean
EOF
}

main() {
    local command="${1:-help}"

    case "$command" in
        build)
            shift
            exec "$SCRIPT_DIR/build.sh" "$@"
            ;;
        utest)
            shift
            exec "$SCRIPT_DIR/utest.sh" "$@"
            ;;
        itest)
            shift
            exec "$SCRIPT_DIR/itest.sh" "$@"
            ;;
        run)
            shift
            exec "$SCRIPT_DIR/run.sh" "$@"
            ;;
        pack)
            shift
            exec "$SCRIPT_DIR/pack.sh" "$@"
            ;;
        cppcheck)
            shift
            exec "$SCRIPT_DIR/cppcheck.sh" "$@"
            ;;
        tidy)
            shift
            "$SCRIPT_DIR/build.sh" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
            exec "$SCRIPT_DIR/clang_tidy.sh" "$@"
            ;;
        cov)
            shift
            exec "$SCRIPT_DIR/cov.sh" "$@"
            ;;
        build-emulator)
            shift
            exec "$SCRIPT_DIR/build_ups_emulator.sh" "$@"
            ;;
        clean)
            shift
            exec "$SCRIPT_DIR/clean.sh" "$@"
            ;;
        help|-h|--help)
            usage
            ;;
        *)
            echo "Unknown command: $command" >&2
            echo >&2
            usage >&2
            exit 1
            ;;
    esac
}

main "$@"
