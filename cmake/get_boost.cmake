if( TARGET Boost::boost )
    return()
endif()

if( MSVC )
    # https://github.com/boostorg/system/issues/32
    add_definitions( -DHAVE_SNPRINTF )
endif()

find_package( Boost COMPONENTS system log log_setup locale REQUIRED )
target_compile_definitions(
        Boost::log
        INTERFACE
        -DBOOST_LOG_DYN_LINK=0
)

add_library(
        Boost::stacktrace INTERFACE IMPORTED
)

if( UNIX )
    if( NOT CMAKE_ADDR2LINE )
        message( WARNING "add2line not found, stacktraces will be without symbol resolution" )
        target_link_libraries( Boost::stacktrace INTERFACE Boost::stacktrace_basic )
    else()
        find_package( Boost COMPONENTS stacktrace_addr2line REQUIRED )
        target_link_libraries( Boost::stacktrace INTERFACE Boost::stacktrace_addr2line )
        target_compile_definitions(
                Boost::stacktrace
                INTERFACE
                -DBOOST_STACKTRACE_USE_ADDR2LINE
                -DBOOST_STACKTRACE_ADDR2LINE_LOCATION=${CMAKE_ADDR2LINE}
        )
        target_link_libraries(
                Boost::stacktrace
                INTERFACE
                dl
        )
    endif()
endif()
