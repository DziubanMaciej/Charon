set(TARGET_NAME CharonDaemon)
add_executable(${TARGET_NAME} WIN32 CMakeLists.txt)
target_common_setup(${TARGET_NAME} Charon)
target_add_sources(${TARGET_NAME} daemon_main.cpp)
target_link_libraries(${TARGET_NAME} PUBLIC CharonLib CharonResources)

set(TARGET_NAME Charon)
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_common_setup(${TARGET_NAME} Charon)
target_add_sources(${TARGET_NAME} console_main.cpp)
target_link_libraries(${TARGET_NAME} PUBLIC CharonLib CharonResources)
add_dependencies(${TARGET_NAME} CharonDaemon)
