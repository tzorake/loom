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

add_executable(test_tzoverload
    test_tzoverload.cpp
)
target_link_libraries(test_tzoverload PRIVATE
    loom-core
    GTest::gtest_main
)
gtest_discover_tests(test_tzoverload)

foreach(impl IN ITEMS separate packed)
    string(TOUPPER "${impl}" IMPL_UPPER)

    add_executable(test_tzbigint_${impl}
        test_tzbigint.cpp
    )
    target_compile_definitions(test_tzbigint_${impl} PRIVATE
        TZBIGINT_IMPL=TZBIGINT_IMPL_${IMPL_UPPER}
    )
    target_link_libraries(test_tzbigint_${impl} PRIVATE 
        loom-core
        GTest::gtest_main
    )
    gtest_discover_tests(test_tzbigint_${impl} TEST_PREFIX "${impl}/")

    add_executable(test_tzfixed_${impl}
        test_tzfixed.cpp
    )
    target_compile_definitions(test_tzfixed_${impl} PRIVATE
        TZBIGINT_IMPL=TZBIGINT_IMPL_${IMPL_UPPER}
    )
    target_link_libraries(test_tzfixed_${impl} PRIVATE 
        loom-core
        GTest::gtest_main
    )
    gtest_discover_tests(test_tzfixed_${impl} TEST_PREFIX "${impl}/")
endforeach()
