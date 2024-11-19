if( TARGET gsl-lite::gsl-lite )
    return()
endif()

include( ./dl_unpack )

set( _gsl_lite_version "0.42.0" )
dl_unpack(
        URL https://github.com/gsl-lite/gsl-lite/archive/refs/tags/v${_gsl_lite_version}.tar.gz
        FILENAME gsl-lite-v${_gsl_lite_version}.tar.gz
        TEST_DIR gsl-lite-${_gsl_lite_version}
)

add_library( gsl-lite::gsl-lite INTERFACE IMPORTED )
target_include_directories( gsl-lite::gsl-lite SYSTEM BEFORE INTERFACE "${EXTERNAL_SRC_ROOT}/gsl-lite-${_gsl_lite_version}/include" )
target_compile_definitions(
        gsl-lite::gsl-lite INTERFACE
        -Dgsl_FEATURE_WITH_CONTAINER_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_MAKE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_BYTE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_CONFIG_NOT_NULL_EXPLICIT_CTOR=1
        -Dgsl_CONFIG_CONTRACT_VIOLATION_CALLS_HANDLER
        -Dgsl_CPLUSPLUS=201703L
)
