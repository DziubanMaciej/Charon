file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

set(TARGET_NAME CharonUnitTests)
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} PRIVATE CharonLib)
target_link_libraries(${TARGET_NAME} PRIVATE gtest)
add_test(NAME UnitTests COMMAND ${TARGET_NAME})
