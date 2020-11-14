if( NOT Python3_EXECUTABLE )
    find_program(
            Python3_EXECUTABLE
            NAMES python3 python
            HINTS ENV Python3_ROOT_DIR
    )
    set( Python3_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "Python interpreter path" )

    if( NOT Python3_EXECUTABLE )
        message( FATAL_ERROR "Python executable not found" )
    endif()
endif()

macro( _python3_get_config_var varname name )
    execute_process(
            COMMAND ${Python3_EXECUTABLE}
            -c "import sysconfig; print(sysconfig.get_config_var('${name}'))"
            OUTPUT_VARIABLE Python3_${varname}
            RESULT_VARIABLE _Python3_EXIT_CODE
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if( _Python3_EXIT_CODE )
        message( FATAL_ERROR "Failed to get config variable ${name} from ${Python3_EXECUTABLE}" )
    endif()
endmacro()

if( NOT Python3_VERSION )
    _python3_get_config_var( VERSION py_version )
    set( Python3_VERSION "${Python3_VERSION}" CACHE INTERNAL "Python interpreter version" )
    if( Python3_VERSION VERSION_LESS 3.6 )
        message( FATAL_ERROR "Incompatible Python version ${Python3_VERSION} found" )
    else()
        message( STATUS "Found Python ${Python3_VERSION} interpreter" )
    endif()
endif()

if( NOT Python3_INCLUDE_DIR )
    _python3_get_config_var( INCLUDE_DIR INCLUDEPY )
    set( Python3_INCLUDE_DIR "${Python3_INCLUDE_DIR}" CACHE PATH "Python include directory" )
endif()

if( NOT Python3_LIBRARY )

    if( WIN32 )
        _python3_get_config_var( LIB_DIR prefix )
        set( Python3_LIB_DIR "${Python3_LIB_DIR}/libs" )
    endif()

    if( VCPKG_TOOLCHAIN )
        # vcpkg tends to be incredibly aggressive linking in its own (bad) library
        set( _old_prefix_path "${CMAKE_PREFIX_PATH}" )
        set( CMAKE_PREFIX_PATH "${Python3_LIB_DIR}" )
    endif()

    if( WIN32 )
        _python3_get_config_var( SHORT_VERSION VERSION )
        find_library(
                Python3_LIBRARY
                NAMES
                python${Python3_SHORT_VERSION} python3 python
        )
    else()
        _python3_get_config_var( LIBPL LIBPL )
        _python3_get_config_var( LDLIBRARY LDLIBRARY )
        set( Python3_LIBRARY "${Python3_LIBPL}/${Python3_LDLIBRARY}" )
    endif()

    if( VCPKG_TOOLCHAIN )
        set( CMAKE_PREFIX_PATH "${_old_prefix_path}" )
        unset( _old_prefix_path )
    endif()

    if( NOT Python3_LIBRARY )
        message( FATAL_ERROR "Python 3 Library not found" )
    endif()
    set( Python3_LIBRARY "${Python3_LIBRARY}" CACHE FILEPATH "Python library" )
endif()

message( STATUS "Found Python ${Python3_VERSION}" )

add_library(
        Python3::Python
        UNKNOWN IMPORTED
)
set_property(
        TARGET Python3::Python
        PROPERTY IMPORTED_LOCATION ${Python3_LIBRARY}
)
target_include_directories(
        Python3::Python
        INTERFACE ${Python3_INCLUDE_DIR}
)

add_executable(
        Python3::Interpreter
        IMPORTED
)
set_property(
        TARGET Python3::Interpreter
        PROPERTY IMPORTED_LOCATION ${Python3_EXECUTABLE}
)
