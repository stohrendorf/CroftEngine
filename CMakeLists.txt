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

include( GNUInstallDirs )

include( get_opengl )
include( get_glm )
include( get_boost )
include( get_ffmpeg )
include( get_cimg )
include( dl_unpack )
include( get_python3 )
include( get_pybind11 )
include( get_gsllite )
include( get_openal )
include( get_utf8cpp )
include( get_gettext )

add_subdirectory( 3rdparty/rapidyaml EXCLUDE_FROM_ALL )
add_subdirectory( 3rdparty/type_safe EXCLUDE_FROM_ALL )
add_subdirectory( src )

if( UNIX )
    install(
            FILES share/logo_256.png
            DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/pixmaps
            RENAME edisonengine.png
    )
    install(
            FILES share/edisonengine.desktop
            DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications
    )
endif()

if( WIN32 )
    add_custom_target(
            prepare-release
            COMMENT Building release dependencies...
            DEPENDS edisonengine edisonengine-python3-deps edisonengine-runtime-deps
    )
endif()