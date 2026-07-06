#!/usr/bin/env bash
#
# project.sh — единая точка входа в инфраструктуру проекта
#
# Назначение:
#   Маршрутизирует пользовательские команды к специализированным скриптам.
#
# Использование:
#   ./scripts/project.sh build
#   ./scripts/project.sh test
#   ./scripts/project.sh run
#   ./scripts/project.sh clean
#   ./scripts/project.sh help
#
# Автор: BGM
#

set -eEuo pipefail
trap 'echo "[ERROR] ${BASH_SOURCE[0]}:${LINENO}: \"${BASH_COMMAND}\" failed" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

usage() {
cat <<'EOF'
Usage:
  ./scripts/project.sh <command> [args]

Commands:
  build    Build project
  test     Run tests
  run      Run application
  cov      Run coverage report
  clean    Remove build artifacts
  cppcheck Run static analysis with Cppcheck
  tidy     Run static analysis with clang-tidy
  help     Show this help

Examples:
  ./scripts/project.sh build
  ./scripts/project.sh test
  ./scripts/project.sh run
  ./scripts/project.sh cov
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
        test)
            shift
            exec "$SCRIPT_DIR/test.sh" "$@"
            ;;
        run)
            shift
            exec "$SCRIPT_DIR/run.sh" "$@"
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
