set(TARGET_NAME CharonLib)
add_library(${TARGET_NAME} STATIC CMakeLists.txt)
target_common_setup(${TARGET_NAME} Charon)
target_find_sources_and_add(${TARGET_NAME})
target_link_libraries(${TARGET_NAME} PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
if (WIN32)
    target_link_libraries(${TARGET_NAME} PUBLIC Comctl32.lib)
endif()
add_subdirectories()
target_setup_vs_folders(${TARGET_NAME})
