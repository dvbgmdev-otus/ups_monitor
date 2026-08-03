# ==========================================
# @file src/snmp/CMakeLists.txt
# Build SNMP library
# ==========================================

set(SNMP_SRC
    ber/snmp_ber_reader.cpp
    ber/snmp_ber_utils.cpp
    ber/snmp_ber_writer.cpp
    snmp_client.cpp
    snmp_codec.cpp
)

add_library(${SNMP_NAME} ${SNMP_SRC})

set_target_properties(${SNMP_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${SNMP_NAME} PUBLIC
    ${PROJECT_INCLUDE_DIR}
)

configure_compile_target(${SNMP_NAME})
