# ==========================================
# @file src/utils/CMakeLists.txt
# Build utils library
# ==========================================

set(UTILS_SRC
    fs_utils.cpp
    ini_section_reader.cpp
    string_utils.cpp
)

add_library(${UTILS_NAME} STATIC ${UTILS_SRC})

target_include_directories(${UTILS_NAME} PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

configure_compile_target(${UTILS_NAME})
