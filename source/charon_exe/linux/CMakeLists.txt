set(TARGET_NAME Charon)
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_common_setup(${TARGET_NAME} Charon)
target_add_sources(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PUBLIC CharonLib -pthread)

install(TARGETS ${TARGET_NAME})
