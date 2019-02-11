project( qs )

set( CMAKE_CXX_STANDARD 14 )

find_package( Boost COMPONENTS unit_test_framework REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

add_executable( qs_test test.cpp )
add_test( NAME qs_test COMMAND test )
target_link_libraries( qs_test ${Boost_LIBRARIES} )
