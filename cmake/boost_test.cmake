include( get_boost )
include( get_gsllite )

find_package( Boost COMPONENTS unit_test_framework REQUIRED )

macro( add_boost_test name )
    add_executable( ${name} ${ARGN} )
    add_test( NAME ${name} COMMAND ${name} -e stderr )
    target_link_libraries( ${name} Boost::unit_test_framework gsl-lite::gsl-lite )
endmacro()
