add_executable(test_tzeventemitter
    test_tzeventemitter.cpp
)

target_link_libraries(test_tzeventemitter PRIVATE
    loom-core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_tzeventemitter)

add_executable(test_tzglobalstatic
    test_tzglobalstatic.cpp
)

target_link_libraries(test_tzglobalstatic PRIVATE
    loom-core
    GTest::gtest_main
)

gtest_discover_tests(test_tzglobalstatic)
