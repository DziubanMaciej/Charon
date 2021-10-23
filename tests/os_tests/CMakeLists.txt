file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

set(TARGET_NAME CharonOsTests)
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} PRIVATE CharonLib)
target_link_libraries(${TARGET_NAME} PRIVATE gtest)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_target_properties(${TARGET_NAME} PROPERTIES
    FOLDER Tests
    VS_DEBUGGER_COMMAND_ARGUMENTS "--gtest_filter=*"
)
add_test(NAME OsTests COMMAND ${TARGET_NAME})
target_compile_definitions(${TARGET_NAME} PRIVATE -DTEST_DIRECTORY_PATH=L"${CMAKE_BINARY_DIR}/test_files")
