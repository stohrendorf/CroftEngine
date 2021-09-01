if( TARGET pybind11::pybind11 )
    return()
endif()

include( ./dl_unpack )

set( _pybind11_version "2.7.1" )
dl_unpack(
        URL https://github.com/pybind/pybind11/archive/v${_pybind11_version}.tar.gz
        FILENAME pybind11-v${_pybind11_version}.tar.gz
        TEST_DIR pybind11-${_pybind11_version}
        PATCH pybind11-2.7.1.patch
)
set( PYBIND11_FINDPYTHON FALSE )
set( PYBIND11_NOPYTHON TRUE )
add_subdirectory( ${EXTERNAL_SRC_ROOT}/pybind11-${_pybind11_version} )
