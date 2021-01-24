if( TARGET CImg::CImg )
    return()
endif()

include( ./dl_unpack )

if( VCPKG_TOOLCHAIN )
    find_package( CImg REQUIRED )
else()
    dl_unpack( https://framagit.org/dtschump/CImg/-/archive/v.2.8.0/CImg-v.2.8.0.tar.bz2 CImg-v.2.8.0.tar.bz2 CImg-v.2.8.0 )

    add_library( CImg::CImg INTERFACE IMPORTED )
    target_include_directories( CImg::CImg SYSTEM BEFORE INTERFACE "${EXTERNAL_SRC_ROOT}/CImg-v.2.8.0" )
endif()
target_compile_definitions( CImg::CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png )

find_package( PNG REQUIRED )
target_link_libraries( CImg::CImg INTERFACE PNG::PNG )
