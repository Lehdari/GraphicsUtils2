foreach(backend ${GU2_BACKEND_OPTIONS})
    set(test_name "test_windows_${backend}")
    add_executable(${test_name} ${CMAKE_CURRENT_SOURCE_DIR}/test_windows.cpp)
    target_link_libraries(${test_name}
        PUBLIC  GTest::gtest_main
        PUBLIC  gtest
        PUBLIC  gu2_os_${backend}
    )
    set_property(TARGET ${test_name}
        PROPERTY    CXX_STANDARD    20
    )

    gtest_add_tests(TARGET ${test_name}
        TEST_SUFFIX .${backend}
        TEST_LIST   noArgsTests
    )
    set_tests_properties(${noArgsTests}
        PROPERTIES  TIMEOUT 10
    )
    gtest_discover_tests(${test_name})
endforeach()
