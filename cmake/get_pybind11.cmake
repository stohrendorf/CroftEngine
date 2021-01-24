if( TARGET pybind11::pybind11 )
    return()
endif()

include( ./dl_unpack )

dl_unpack( https://github.com/pybind/pybind11/archive/v2.6.0.tar.gz pybind11-v2.6.0.tar.gz pybind11-2.6.0 )
set( PYBIND11_FINDPYTHON FALSE )
set( PYBIND11_NOPYTHON TRUE )
add_subdirectory( ${EXTERNAL_SRC_ROOT}/pybind11-2.6.0 )
