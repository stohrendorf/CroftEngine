# - Try to find ffmpeg libraries (libavcodec, libavformat and libavutil)
# Once done this will define
#
# FFMPEG_FOUND - system has ffmpeg or libav
# FFMPEG_INCLUDE_DIR - the ffmpeg include directory
# FFMPEG_LIBRARIES - Link these to use ffmpeg
# FFMPEG_LIBAVCODEC
# FFMPEG_LIBAVFORMAT
# FFMPEG_LIBAVUTIL
#
# Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
# Modified for other libraries by Lasse Kärkkäinen <tronic>
# Modified for Hedgewars by Stepik777
# Modified for FFmpeg-example Tuukka Pasanen 2018
#
# Redistribution and use is allowed according to the terms of the New
# BSD license.
#

include( FindPackageHandleStandardArgs )

if( FFMPEG_LIBRARY AND FFMPEG_INCLUDE_DIR )
    # in cache already
    set( FFMPEG_FOUND TRUE )
else()
    # use pkg-config to get the directories and then use these values
    # in the FIND_PATH() and FIND_LIBRARY() calls
    find_package( PkgConfig QUIET )
    if( PKG_CONFIG_FOUND )
        pkg_check_modules( _FFMPEG_AVCODEC libavcodec )
        pkg_check_modules( _FFMPEG_AVFORMAT libavformat )
        pkg_check_modules( _FFMPEG_AVUTIL libavutil )
    endif()

    find_path( FFMPEG_AVCODEC_INCLUDE_DIR
               NAMES libavcodec/avcodec.h
               PATHS
               ${_FFMPEG_AVCODEC_INCLUDE_DIRS}
               /usr/include
               /usr/local/include
               /opt/local/include
               /sw/include
               include
               PATH_SUFFIXES ffmpeg libav )

    find_library( FFMPEG_LIBAVCODEC
                  NAMES avcodec
                  PATHS
                  ${_FFMPEG_AVCODEC_LIBRARY_DIRS}
                  /usr/lib
                  /usr/local/lib
                  /opt/local/lib
                  /sw/lib
                  lib )

    find_library( FFMPEG_LIBAVFORMAT
                  NAMES avformat
                  PATHS
                  ${_FFMPEG_AVFORMAT_LIBRARY_DIRS}
                  /usr/lib
                  /usr/local/lib
                  /opt/local/lib
                  /sw/lib
                  lib )

    find_library( FFMPEG_LIBAVUTIL
                  NAMES avutil
                  PATHS
                  ${_FFMPEG_AVUTIL_LIBRARY_DIRS}
                  /usr/lib
                  /usr/local/lib
                  /opt/local/lib
                  /sw/lib
                  lib )

    find_library( FFMPEG_LIBAVFILTER
                  NAMES avfilter
                  PATHS
                  ${_FFMPEG_AVFILTER_LIBRARY_DIRS}
                  /usr/lib
                  /usr/local/lib
                  /opt/local/lib
                  /sw/lib
                  lib )

    find_library( FFMPEG_LIBSWRESAMPLE
                  NAMES swresample
                  PATHS
                  ${_FFMPEG_SWRESAMPLE_LIBRARY_DIRS}
                  /usr/lib
                  /usr/local/lib
                  /opt/local/lib
                  /sw/lib
                  lib )

    find_library( FFMPEG_LIBSWSCALE
                  NAMES swscale
                  PATHS
                  ${_FFMPEG_SWRESAMPLE_LIBRARY_DIRS}
                  /usr/lib
                  /usr/local/lib
                  /opt/local/lib
                  /sw/lib
                  lib )

    if( FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT AND FFMPEG_LIBAVUTIL AND FFMPEG_LIBSWRESAMPLE AND FFMPEG_LIBSWSCALE AND FFMPEG_LIBAVFILTER )
        set( FFMPEG_FOUND TRUE )
    else()
        set( FFMPEG_FOUND FALSE )
    endif()

    if( FFMPEG_FOUND )
        set( FFMPEG_INCLUDE_DIR ${FFMPEG_AVCODEC_INCLUDE_DIR} )
        set( FFMPEG_LIBRARY
             ${FFMPEG_LIBAVCODEC}
             ${FFMPEG_LIBAVFORMAT}
             ${FFMPEG_LIBAVUTIL}
             ${FFMPEG_LIBAVFILTER}
             ${FFMPEG_LIBSWRESAMPLE}
             ${FFMPEG_LIBSWSCALE} )
    endif()
endif()

find_package_handle_standard_args( FFMPEG
                                   FOUND_VAR FFMPEG_FOUND
                                   REQUIRED_VARS FFMPEG_LIBRARY FFMPEG_INCLUDE_DIR
                                   )
