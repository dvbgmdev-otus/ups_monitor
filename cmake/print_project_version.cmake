# ==========================================
# @file /cmake/print_project_version.cmake
# Print project version
# ==========================================
#
# Назначение:
#   Печатает версию проекта для внешних скриптов и CI.
#

include("${CMAKE_CURRENT_LIST_DIR}/project_version.cmake")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E echo "${PROJECT_VERSION_VALUE}"
)
