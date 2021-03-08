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
            "URL;FILENAME;TEST;WORKING_DIRECTORY"
            ""
            ${ARGN}
    )

    if( NOT DL_UNPACK_URL OR NOT DL_UNPACK_FILENAME OR NOT DL_UNPACK_TEST )
        message( FATAL_ERROR "invalid call to dl_unpack" )
    endif()

    download(
            URL ${DL_UNPACK_URL}
            FILENAME ${DL_UNPACK_FILENAME}
            WORKING_DIRECTORY ${DL_UNPACK_WORKING_DIRECTORY}
    )

    if( NOT EXISTS "${EXTERNAL_SRC_ROOT}/${DL_UNPACK_WORKING_DIRECTORY}/${DL_UNPACK_TEST}" )
        message( STATUS "Extracting ${DL_UNPACK_FILENAME}..." )
        execute_process( COMMAND "${CMAKE_COMMAND}" -E tar xfz "${DL_UNPACK_FILENAME}"
                WORKING_DIRECTORY "${EXTERNAL_SRC_ROOT}/${DL_UNPACK_WORKING_DIRECTORY}"
                RESULT_VARIABLE rv )

        if( NOT rv EQUAL 0 )
            message( FATAL_ERROR "Failed to extract ${DL_UNPACK_FILENAME}" )
        endif()
    else()
        message( STATUS "Already extracted ${DL_UNPACK_FILENAME}" )
    endif()
endfunction()
