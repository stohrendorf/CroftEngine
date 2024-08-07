if( TARGET OpenAL )
    return()
endif()

if( VCPKG_TOOLCHAIN )
    find_package( OpenAL CONFIG REQUIRED )
else()
    find_package( OpenAL REQUIRED )
    if( NOT TARGET OpenAL::OpenAL )
        add_library( OpenAL::OpenAL INTERFACE IMPORTED )
        target_include_directories( OpenAL::OpenAL SYSTEM BEFORE INTERFACE ${OPENAL_INCLUDE_DIR} )
        target_link_libraries( OpenAL::OpenAL INTERFACE ${OPENAL_LIBRARY} )
    endif()
endif()
