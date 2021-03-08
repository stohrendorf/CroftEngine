if( TARGET CImg::CImg )
    return()
endif()

include( ./dl_unpack )

if( VCPKG_TOOLCHAIN )
    find_package( CImg REQUIRED )
else()
    set( _cimg_version "v.2.9.4" )
    dl_unpack(
            URL https://github.com/dtschump/CImg/archive/${_cimg_version}.tar.gz
            FILENAME CImg-${_cimg_version}.tar.gz
            TEST CImg-${_cimg_version}
    )

    add_library( CImg::CImg INTERFACE IMPORTED )
    target_include_directories( CImg::CImg SYSTEM BEFORE INTERFACE "${EXTERNAL_SRC_ROOT}/CImg-${_cimg_version}" )
endif()
target_compile_definitions( CImg::CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png )

find_package( PNG REQUIRED )
target_link_libraries( CImg::CImg INTERFACE PNG::PNG )
