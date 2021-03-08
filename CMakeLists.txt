cmake_minimum_required( VERSION 3.16 )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

project( EdisonEngine )

cmake_policy( SET CMP0069 NEW )
cmake_policy( SET CMP0094 NEW )

include( toolchain_config )

function( group_files )
    foreach( fn ${ARGV} )
        get_filename_component( parentDir "${fn}" PATH )

        # change /'s to \\'s, and reduce redundant // or \\.
        string( REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}" )

        source_group( "${grp}" FILES "${fn}" )
    endforeach()
endfunction()

include( get_opengl )
include( get_glm )
include( get_boost )
include( get_sndfile )
include( get_ffmpeg )
include( get_cimg )
include( dl_unpack )
include( get_python3 )
include( get_pybind11 )
include( get_gsllite )
include( get_soloud )
include( get_utf8cpp )
include( get_gettext )

add_subdirectory( 3rdparty/rapidyaml )
add_subdirectory( 3rdparty/type_safe )
add_subdirectory( src )
