include( get_boost )
include( get_gsllite )

find_package( Boost COMPONENTS unit_test_framework REQUIRED )
find_package( Threads REQUIRED )

macro( add_boost_test name )
    add_executable( ${name} ${CMAKE_SOURCE_DIR}/src/gslfailhandler.cpp ${ARGN} )
    target_include_directories( ${name} PRIVATE ${CMAKE_SOURCE_DIR}/src )
    add_test( NAME ${name} COMMAND ${name} -e stderr )
    target_link_libraries(
            ${name}
            PRIVATE
            Boost::system
            Boost::unit_test_framework
            Boost::log
            Boost::disable_autolinking
            Boost::headers
            gsl-lite::gsl-lite
            Threads::Threads
            ${CMAKE_DL_LIBS}
    )
endmacro()
