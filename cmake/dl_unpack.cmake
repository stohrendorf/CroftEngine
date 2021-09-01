if( EXTERNAL_SRC_ROOT )
    return()
endif()

set( EXTERNAL_SRC_ROOT "${CMAKE_SOURCE_DIR}/3rdparty" )

function( download )
    cmake_parse_arguments(
            DOWNLOAD
            "" # no options
            "URL;FILENAME;WORKING_DIRECTORY"
            ""
            ${ARGN}
    )

    if( NOT DOWNLOAD_URL OR NOT DOWNLOAD_FILENAME )
        message( FATAL_ERROR "invalid call to download" )
    endif()

    if( NOT EXISTS "${EXTERNAL_SRC_ROOT}/${DOWNLOAD_WORKING_DIRECTORY}/${DOWNLOAD_FILENAME}" )
        message( STATUS "Downloading ${DOWNLOAD_URL}..." )
        file(
                DOWNLOAD "${DOWNLOAD_URL}" "${EXTERNAL_SRC_ROOT}/${DOWNLOAD_WORKING_DIRECTORY}/${DOWNLOAD_FILENAME}"
                STATUS status
                LOG log
        )
        list( GET status 0 status_code )
        list( GET status 1 status_string )

        if( NOT status_code EQUAL 0 )
            message( FATAL_ERROR "Download failed with code ${status_code}" )
        endif()
    else()
        message( STATUS "Already downloaded ${DOWNLOAD_URL}" )
    endif()
endfunction()

function( dl_unpack )
    cmake_parse_arguments(
            DL_UNPACK
            "" # no options
            "URL;FILENAME;TEST_DIR;TEST_FILE;WORKING_DIRECTORY;PATCH"
            ""
            ${ARGN}
    )

    if( NOT DL_UNPACK_URL OR NOT DL_UNPACK_FILENAME OR ( NOT DL_UNPACK_TEST_DIR AND NOT DL_UNPACK_TEST_FILE ))
        message( FATAL_ERROR "invalid call to dl_unpack" )
    endif()

    download(
            URL ${DL_UNPACK_URL}
            FILENAME ${DL_UNPACK_FILENAME}
            WORKING_DIRECTORY ${DL_UNPACK_WORKING_DIRECTORY}
    )

    if( DL_UNPACK_TEST_DIR )
        set( test ${DL_UNPACK_TEST_DIR} )
    else()
        set( test ${DL_UNPACK_TEST_FILE} )
    endif()
    if( NOT EXISTS "${EXTERNAL_SRC_ROOT}/${DL_UNPACK_WORKING_DIRECTORY}/${test}" )
        message( STATUS "Extracting ${DL_UNPACK_FILENAME}..." )
        execute_process(
                COMMAND "${CMAKE_COMMAND}" -E tar xfz "${DL_UNPACK_FILENAME}"
                WORKING_DIRECTORY "${EXTERNAL_SRC_ROOT}/${DL_UNPACK_WORKING_DIRECTORY}"
                RESULT_VARIABLE rv
        )

        if( NOT rv EQUAL 0 )
            message( FATAL_ERROR "Failed to extract ${DL_UNPACK_FILENAME}" )
        endif()
    else()
        message( STATUS "Already extracted ${DL_UNPACK_FILENAME}" )
    endif()

    if( DL_UNPACK_PATCH )
        if( NOT DL_UNPACK_TEST_DIR )
            message( FATAL_ERROR "PATCH required DL_UNPACK_TEST_DIR" )
        endif()

        find_package( Git REQUIRED )

        message( STATUS "Apply patch ${DL_UNPACK_PATCH} to ${DL_UNPACK_FILENAME}..." )
        execute_process(
                COMMAND
                ${GIT_EXECUTABLE}
                apply
                --unsafe-paths
                -p1
                "--directory=${DL_UNPACK_TEST_DIR}"
                -v
                --reject
                "${EXTERNAL_SRC_ROOT}/${DL_UNPACK_PATCH}"
                WORKING_DIRECTORY "${EXTERNAL_SRC_ROOT}"
        )
    endif()
endfunction()
