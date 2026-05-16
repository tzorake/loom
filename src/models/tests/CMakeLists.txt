add_executable(test_eventemitter
    test_eventemitter.cpp
)

add_executable(test_tztreemodel
    test_tztreemodel.cpp
)

add_executable(test_tzbreadcrumbmodel
    test_tzbreadcrumbmodel.cpp
)

target_link_libraries(test_eventemitter PRIVATE
    models
    GTest::gtest_main
)

target_link_libraries(test_tztreemodel PRIVATE
    models
    GTest::gtest_main
)

target_link_libraries(test_tzbreadcrumbmodel PRIVATE
    models
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_eventemitter)
gtest_discover_tests(test_tztreemodel)
gtest_discover_tests(test_tzbreadcrumbmodel)
