file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

set(TARGET_NAME CharonLib)
add_library(${TARGET_NAME} STATIC ${SOURCE_FILES})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(${TARGET_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)
set_target_properties(CharonLib PROPERTIES FOLDER Charon)
if (WIN32)
    target_compile_definitions(${TARGET_NAME} PUBLIC -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()
