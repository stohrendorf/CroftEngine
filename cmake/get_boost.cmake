if( TARGET Boost::boost )
    return()
endif()

if( MSVC )
    # https://github.com/boostorg/system/issues/32
    add_definitions( -DHAVE_SNPRINTF )
endif()

find_package( Boost COMPONENTS system log locale REQUIRED )
add_definitions( -DBOOST_ALL_NO_LIB -DBOOST_LOG_DYN_LINK=0 )
