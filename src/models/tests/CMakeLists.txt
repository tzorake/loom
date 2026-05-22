add_executable(test_tzabstractitemmodel
    test_tzabstractitemmodel.cpp
)

target_link_libraries(test_tzabstractitemmodel PRIVATE
    loom-models
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_tzabstractitemmodel)

add_executable(test_tzabstractproxymodel
    test_tzabstractproxymodel.cpp
)

target_link_libraries(test_tzabstractproxymodel PRIVATE
    loom-models
    GTest::gtest_main
)

gtest_discover_tests(test_tzabstractproxymodel)

add_executable(test_tzitemselectionmodel
    test_tzitemselectionmodel.cpp
)

target_link_libraries(test_tzitemselectionmodel PRIVATE
    loom-models
    GTest::gtest_main
)

gtest_discover_tests(test_tzitemselectionmodel)
