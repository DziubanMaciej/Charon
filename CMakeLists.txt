cmake_minimum_required(VERSION 3.16.0)

project(Charon)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
include(CMakeUtils.cmake)

add_subdirectory(third_party)
add_subdirectory(source)

enable_testing()
add_subdirectory(tests)
