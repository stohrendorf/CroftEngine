include( FindPackageHandleStandardArgs )

if( NOT Python3_EXECUTABLE )
    find_program(
            Python3_EXECUTABLE
            NAMES python3 python
            DOC "Python interpreter path"
    )

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

if( WIN32 )
    _python3_get_config_var( INSTALLED_BASE installed_base )
    _python3_get_config_var( VERSION_NODOT py_version_nodot )
    _python3_get_config_var( LIBDEST LIBDEST )

    set( Python3_DLL ${Python3_INSTALLED_BASE}/python${Python3_VERSION_NODOT}.dll )
    if( NOT EXISTS ${Python3_DLL} )
        message( FATAL_ERROR "Could not find Python DLL ${Python3_DLL}" )
    endif()

    file(
            GLOB_RECURSE Python3_LIB_FILES
            RELATIVE ${Python3_INSTALLED_BASE}
            ${Python3_LIBDEST}/*
    )
    list( FILTER Python3_LIB_FILES EXCLUDE REGEX "__pycache__|site-packages|/tests?/|/test_|_test\.|\.pyc\$" )
endif()

function( copy_python3_deps target )
    if( WIN32 )
        get_target_property( _bin_dir ${target} BINARY_DIR )

        add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMENT "Copy ${Python3_DLL}"
                COMMAND ${CMAKE_COMMAND} -E copy ${Python3_DLL} ${_bin_dir}
        )

        add_custom_target(
                ${target}-python3-deps
                COMMENT "Copy Python3 lib dir"
        )

        foreach( _file ${Python3_LIB_FILES} )
            get_filename_component( _dirname ${_file} DIRECTORY )
            add_custom_command(
                    TARGET ${target}-python3-deps
                    POST_BUILD
                    COMMENT "Copy ${_file}"
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${_bin_dir}/${_dirname}
                    COMMAND ${CMAKE_COMMAND} -E copy ${Python3_INSTALLED_BASE}/${_file} ${_bin_dir}/${_dirname}/
            )
        endforeach()
    endif()
endfunction()

find_package_handle_standard_args(
        Python3
        REQUIRED_VARS Python3_EXECUTABLE Python3_LIBRARY Python3_INCLUDE_DIR
        VERSION_VAR Python3_VERSION
)
