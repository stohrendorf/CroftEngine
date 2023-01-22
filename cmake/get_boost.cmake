if( TARGET Boost::boost )
    return()
endif()

if( MSVC )
    # https://github.com/boostorg/system/issues/32
    add_definitions( -DHAVE_SNPRINTF )
endif()

find_package( Boost COMPONENTS system log log_setup locale iostreams REQUIRED )

if( UNIX )
    target_compile_definitions( Boost::log INTERFACE -DBOOST_LOG_DYN_LINK )
endif()
