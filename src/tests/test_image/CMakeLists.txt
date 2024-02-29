add_executable(test_image ${CMAKE_CURRENT_SOURCE_DIR}/test_image.cpp)
target_link_libraries(test_image
    PUBLIC  GTest::gtest_main
    PUBLIC  gtest
    PUBLIC  gu2_util
)
set_property(TARGET test_image
    PROPERTY    CXX_STANDARD    20
)

gtest_add_tests(TARGET test_image
    TEST_SUFFIX .noArgs
    TEST_LIST   noArgsTests
)
set_tests_properties(${noArgsTests}
    PROPERTIES  TIMEOUT 10
)
gtest_discover_tests(test_image)
