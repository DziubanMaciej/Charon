set(SOURCE_FILES
    daemon_main.cpp
    tmp_test_config.json
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
set(TARGET_NAME CharonDaemon)
add_executable(${TARGET_NAME} WIN32 ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} PUBLIC CharonLib CharonResources)
set_target_properties(${TARGET_NAME} PROPERTIES
    FOLDER Charon
    VS_DEBUGGER_COMMAND_ARGUMENTS "--config ${CMAKE_CURRENT_SOURCE_DIR}/tmp_test_config.json "
)

set(SOURCE_FILES
    console_main.cpp
    tmp_test_config.json
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
set(TARGET_NAME Charon)
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} PUBLIC CharonLib CharonResources)
set_target_properties(${TARGET_NAME} PROPERTIES 
    FOLDER Charon
    VS_DEBUGGER_COMMAND_ARGUMENTS "--config ${CMAKE_CURRENT_SOURCE_DIR}/tmp_test_config.json "
)
add_dependencies(${TARGET_NAME} CharonDaemon)
