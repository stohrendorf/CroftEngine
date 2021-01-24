include( get_boost )
find_package( Boost COMPONENTS unit_test_framework REQUIRED )
include( get_gsllite )

add_executable( qs_test test.cpp )
add_test( NAME qs_test COMMAND test )
target_link_libraries( qs_test Boost::unit_test_framework gsl-lite::gsl-lite )
