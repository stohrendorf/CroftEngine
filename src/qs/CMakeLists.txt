project( qs )

set( CMAKE_CXX_STANDARD 14 )

find_package( Boost COMPONENTS unit_test_framework REQUIRED )
include_directories( ${Boost_INCLUDE_DIR} )

add_executable( test_tuple test_tuple.cpp )
add_test( NAME test_tuple COMMAND test_tuple )
target_link_libraries( test_tuple ${Boost_LIBRARIES} )
add_executable( test_mult_div test_mult_div.cpp )
add_test( NAME test_mult_div COMMAND test_mult_div )
target_link_libraries( test_mult_div ${Boost_LIBRARIES} )
