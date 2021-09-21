include( get_boost )
include( get_gsllite )

find_package( Boost COMPONENTS unit_test_framework REQUIRED )
find_package( Threads REQUIRED )

macro( add_boost_test name )
    add_executable(
            ${name}
            ${CMAKE_SOURCE_DIR}/src/gslfailhandler.cpp
            ${CMAKE_SOURCE_DIR}/src/testutil/teamcityformatter.cpp
            ${CMAKE_SOURCE_DIR}/src/testutil/teamcitymessages.cpp
            ${ARGN}
    )
    target_include_directories( ${name} PRIVATE ${CMAKE_SOURCE_DIR}/src )
    add_test( NAME ${name} COMMAND ${name} )
    target_link_libraries(
            ${name}
            PRIVATE
            Boost::system
            Boost::unit_test_framework
            Boost::log
            Boost::log_setup
            Boost::disable_autolinking
            Boost::headers
            gsl-lite::gsl-lite
            Threads::Threads
            glm::glm
            ${CMAKE_DL_LIBS}
    )
    target_compile_definitions(
            ${name}
            PRIVATE
            -DBOOST_TEST_DYN_LINK
    )
endmacro()
