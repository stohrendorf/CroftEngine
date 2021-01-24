if( TARGET pybind11::pybind11 )
    return()
endif()

include( ./dl_unpack )

set( _pybind11_version "2.6.1" )
dl_unpack( https://github.com/pybind/pybind11/archive/v${_pybind11_version}.tar.gz pybind11-v${_pybind11_version}.tar.gz pybind11-${_pybind11_version} )
set( PYBIND11_FINDPYTHON FALSE )
set( PYBIND11_NOPYTHON TRUE )
add_subdirectory( ${EXTERNAL_SRC_ROOT}/pybind11-${_pybind11_version} )
