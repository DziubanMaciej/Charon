cmake_minimum_required(VERSION 3.16.0)

project(Charon)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
include(CMakeUtils.cmake)

add_subdirectory(third_party)
add_subdirectory(source)

enable_testing()
add_subdirectory(tests)
