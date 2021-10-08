cmake_minimum_required(VERSION 3.16.0)

project(Charon)

add_subdirectory(third_party)
add_subdirectory(source)

enable_testing()
add_subdirectory(tests)
