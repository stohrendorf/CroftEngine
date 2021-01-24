if( TARGET gsl-lite::gsl-lite )
    return()
endif()

include( ./dl_unpack )

download( https://raw.githubusercontent.com/martinmoene/gsl-lite/4b796627ad0fa42640f5fdb96f23c4a0d9ee084f/include/gsl/gsl-lite.hpp gsl-lite/gsl-lite.hpp )

add_library( gsl-lite::gsl-lite INTERFACE IMPORTED )
target_include_directories( gsl-lite::gsl-lite SYSTEM BEFORE INTERFACE "${EXTERNAL_SRC_ROOT}/gsl-lite" )
target_compile_definitions(
        gsl-lite::gsl-lite INTERFACE
        -Dgsl_FEATURE_WITH_CONTAINER_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_MAKE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_BYTE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_CONFIG_NOT_NULL_EXPLICIT_CTOR=0
        -Dgsl_CONFIG_CONTRACT_VIOLATION_CALLS_HANDLER
)
