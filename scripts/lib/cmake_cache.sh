#!/usr/bin/env bash
#
# cmake_cache.sh — проверка параметров конфигурации в CMakeCache.txt
#
# Использование:
#   source scripts/lib/cmake_cache.sh
#
#   cmake_cache_matches \
#       build/CMakeCache.txt \
#       BUILD_TESTING BOOL ON \
#       BUILD_COVERAGE BOOL OFF

# Предотвращаем выполнение этого файла напрямую
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf "This script is a library and should not be executed directly\n" >&2
    exit 1
fi

# Защита от повторного включения
if [[ -z "${__CMAKE_CACHE_SH_INCLUDED:-}" ]]; then
    readonly __CMAKE_CACHE_SH_INCLUDED=1

    # Внутренний helper: проверяет соответствие одной записи CMake-кэша
    # ожидаемому значению.
    #
    # Аргументы:
    #   $1 — путь к CMakeCache.txt
    #   $2 — имя переменной
    #   $3 — тип переменной CMake
    #   $4 — ожидаемое значение
    _cmake_cache_entry_matches() {
        local cache_file="$1"
        local variable_name="$2"
        local variable_type="$3"
        local expected_value="$4"

        [[ -f "$cache_file" ]] || return 1

        grep -Fqx \
            "${variable_name}:${variable_type}=${expected_value}" \
            "$cache_file"
    }

    # Проверяет соответствие одной или нескольких записей CMake-кэша.
    #
    # После пути к CMakeCache.txt аргументы передаются тройками:
    #   <имя> <тип> <значение>
    cmake_cache_matches() {
        local cache_file="$1"
        shift

        if (( $# == 0 || $# % 3 != 0 )); then
            return 2
        fi

        while (( $# > 0 )); do
            if ! _cmake_cache_entry_matches "$cache_file" "$1" "$2" "$3"; then
                return 1
            fi
            shift 3
        done
    }
fi
