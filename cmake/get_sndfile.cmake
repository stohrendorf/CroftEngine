if( TARGET SndFile::sndfile )
    return()
endif()

if( VCPKG_TOOLCHAIN )
    find_package( SndFile REQUIRED )
else()
    find_path( LIBSNDFILE_INCLUDE_DIRS sndfile.h )
    find_library( LIBSNDFILE_LIBRARIES sndfile )

    if( NOT LIBSNDFILE_INCLUDE_DIRS OR NOT LIBSNDFILE_LIBRARIES )
        message( FATAL_ERROR "Could not find libsndfile" )
    endif()

    add_library( SndFile::sndfile INTERFACE IMPORTED )
    target_include_directories( SndFile::sndfile SYSTEM BEFORE INTERFACE "${LIBSNDFILE_INCLUDE_DIRS}" )
    target_link_libraries( SndFile::sndfile INTERFACE "${LIBSNDFILE_LIBRARIES}" )
endif()
