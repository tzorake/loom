add_executable(test_tzeventemitter
    test_tzeventemitter.cpp
)

target_link_libraries(test_tzeventemitter PRIVATE
    loom-core
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_tzeventemitter)
