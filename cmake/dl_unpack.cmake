if( EXTERNAL_SRC_ROOT )
    return()
endif()

set( EXTERNAL_SRC_ROOT "${CMAKE_SOURCE_DIR}/3rdparty" )

function( download URL FILENAME )
    if( NOT EXISTS "${EXTERNAL_SRC_ROOT}/${FILENAME}" )
        message( STATUS "Downloading ${URL}..." )
        file( DOWNLOAD "${URL}" "${EXTERNAL_SRC_ROOT}/${FILENAME}"
                STATUS status
                LOG log )
        list( GET status 0 status_code )
        list( GET status 1 status_string )

        if( NOT status_code EQUAL 0 )
            message( FATAL_ERROR "Download failed with code ${status_code}" )
        endif()
    else()
        message( STATUS "Already downloaded ${URL}" )
    endif()
endfunction()

function( dl_unpack URL ARCHIVE_NAME DIRNAME )
    download( "${URL}" "${ARCHIVE_NAME}" )

    if( NOT EXISTS "${EXTERNAL_SRC_ROOT}/${DIRNAME}" )
        message( STATUS "Extracting ${ARCHIVE_NAME}..." )
        execute_process( COMMAND "${CMAKE_COMMAND}" -E tar xfz "${ARCHIVE_NAME}"
                WORKING_DIRECTORY "${EXTERNAL_SRC_ROOT}"
                RESULT_VARIABLE rv )

        if( NOT rv EQUAL 0 )
            message( FATAL_ERROR "Failed to extract ${ARCHIVE_NAME}" )
        endif()
    else()
        message( STATUS "Already extracted ${ARCHIVE_NAME}" )
    endif()
endfunction()
