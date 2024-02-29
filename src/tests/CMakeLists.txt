enable_testing()
find_package(GTest REQUIRED)

add_subdirectory(test_windows)

# required for "test" target
add_custom_target(test COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS ${GU2_TEST_EXECUTABLES})
