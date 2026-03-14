include( boost_test )
add_boost_test( serialization_test
        tests/test_main.cpp
        tests/test_basic.cpp
        tests/test_containers.cpp
        tests/test_math.cpp
        tests/test_misc.cpp
        tests/test_ptr.cpp
)
target_link_libraries( serialization_test PRIVATE serialization soglb )
