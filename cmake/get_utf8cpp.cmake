if( TARGET utf8cpp::utf8cpp )
    return()
endif()

find_path(
        UTF8CPP_INCLUDE_DIR
        NAMES utf8.h
        PATH_SUFFIXES utf8cpp
)
if( NOT UTF8CPP_INCLUDE_DIR )
    message( FATAL_ERROR "utf8cpp not found" )
endif()
add_library( utf8cpp::utf8cpp INTERFACE IMPORTED )
target_include_directories( utf8cpp::utf8cpp INTERFACE "${UTF8CPP_INCLUDE_DIR}" )
