if( TARGET CImg::CImg )
    return()
endif()

include( ./dl_unpack )

set( _cimg_version "v.3.3.0" )
dl_unpack(
        URL https://github.com/GreycLab/CImg/archive/${_cimg_version}.tar.gz
        FILENAME CImg-${_cimg_version}.tar.gz
        TEST_DIR CImg-${_cimg_version}
)

add_library( CImg::CImg INTERFACE IMPORTED )
target_include_directories( CImg::CImg SYSTEM BEFORE INTERFACE "${EXTERNAL_SRC_ROOT}/CImg-${_cimg_version}" )
target_compile_definitions( CImg::CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png -Dcimg_use_jpeg )

find_package( PNG REQUIRED )
target_link_libraries( CImg::CImg INTERFACE PNG::PNG )

find_package( JPEG REQUIRED )
target_include_directories( CImg::CImg INTERFACE ${JPEG_INCLUDE_DIR} )
target_link_libraries( CImg::CImg INTERFACE ${JPEG_LIBRARIES} )