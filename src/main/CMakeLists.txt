# ==========================================
# @file src/main/CMakeLists.txt
# Build bulk executable
# ==========================================

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CORE_NAME}
)

configure_compile_target(${PROJECT_NAME})
configure_link_target(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
