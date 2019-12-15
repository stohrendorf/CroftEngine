find_package( Boost COMPONENTS unit_test_framework REQUIRED )

add_executable( qs_test test.cpp )
add_test( NAME qs_test COMMAND test )
target_link_libraries( qs_test Boost::boost gsl-lite )
