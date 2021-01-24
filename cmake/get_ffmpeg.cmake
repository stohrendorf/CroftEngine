if( TARGET FFmpeg )
    return()
endif()

find_package( FFmpeg REQUIRED )
add_library( FFmpeg INTERFACE )
target_include_directories( FFmpeg SYSTEM BEFORE INTERFACE ${FFMPEG_INCLUDE_DIR} )
target_link_libraries( FFmpeg INTERFACE ${FFMPEG_LIBRARY} )
