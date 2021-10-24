function(target_common_setup TARGET_NAME VS_FOLDER)
    set_target_properties(${TARGET_NAME} PROPERTIES LINKER_LANGUAGE CXX)

    if (MSVC)
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER ${VS_FOLDER})
        target_compile_definitions(${TARGET_NAME} PRIVATE -DNOMINMAX -DWIN32_LEAN_AND_MEAN)
    endif()

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    target_compile_options(${TARGET_NAME} PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:/W4>
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
    )
endfunction()

function(target_add_sources TARGET_NAME)
    set(SOURCE_FILES ${ARGN})
    target_sources(${TARGET_NAME} PRIVATE ${SOURCE_FILES})
endfunction()

function(target_setup_vs_folders TARGET_NAME)
    get_target_property(SOURCES ${TARGET_NAME} SOURCES)
    get_target_property(SOURCE_DIR ${TARGET_NAME} SOURCE_DIR)
    source_group(TREE ${SOURCE_DIR} FILES ${SOURCES})
endfunction()

function(target_find_sources_and_add TARGET_NAME)
    file(GLOB SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/*.inl
        ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    )
    target_add_sources(${TARGET_NAME} ${SOURCE_FILES})
endfunction()

macro(add_subdirectories)
    set(FORBIDDEN_DIR_NAMES)
    if (UNIX)
        list(APPEND FORBIDDEN_DIR_NAMES "windows")
    elseif (WIN32)
        list(APPEND FORBIDDEN_DIR_NAMES "linux")
    endif()

    file(GLOB SUB_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(SUB_DIR ${SUB_DIRS})
        if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_DIR}/CMakeLists.txt)
            continue()
        endif()

        list(FIND FORBIDDEN_DIR_NAMES ${SUB_DIR} INDEX)
        if (NOT INDEX EQUAL -1)
            continue()
        endif()

        add_subdirectory(${SUB_DIR})
    endforeach()
endmacro()
