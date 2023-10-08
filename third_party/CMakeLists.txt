if(CHARON_TESTS)
    set(BUILD_GMOCK ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(googletest)
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER ThirdParty)

    add_subdirectory(oakum)
    set_target_properties(Oakum PROPERTIES FOLDER ThirdParty)
endif()

add_subdirectory(json)
