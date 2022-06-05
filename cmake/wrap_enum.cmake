include( get_python3 )

function( wrap_enum )
    cmake_parse_arguments(
            WRAP_ENUM
            "" # no options
            "NAMESPACE;NAME;TYPE;SOURCE;DEST;DEST_IMPL"
            "WITH_INCLUDES"
            ${ARGN}
    )

    if( WRAP_ENUM_UNPARSED_ARGUMENTS )
        message( FATAL_ERROR "Unexpected ${WRAP_ENUM_UNPARSED_ARGUMENTS}" )
    endif()
    if( WRAP_ENUM_KEYWORDS_MISSING_VALUES )
        message( FATAL_ERROR "Missing ${WRAP_ENUM_KEYWORDS_MISSING_VALUES}" )
    endif()

    if(
            EXISTS "${WRAP_ENUM_DEST}" AND "${WRAP_ENUM_DEST}" IS_NEWER_THAN "${WRAP_ENUM_SOURCE}"
            AND EXISTS "${WRAP_ENUM_DEST_IMPL}" AND "${WRAP_ENUM_DEST_IMPL}" IS_NEWER_THAN "${WRAP_ENUM_SOURCE}"
    )
        return()
    endif()

    add_custom_command(
            OUTPUT ${WRAP_ENUM_DEST} ${WRAP_ENUM_DEST_IMPL}
            COMMAND
            Python3::Interpreter
            ${CMAKE_SOURCE_DIR}/cmake/scripts/wrap_enum.py
            --namespace ${WRAP_ENUM_NAMESPACE}
            --name ${WRAP_ENUM_NAME}
            --type ${WRAP_ENUM_TYPE}
            --source ${WRAP_ENUM_SOURCE}
            --dest ${WRAP_ENUM_DEST}
            --dest-impl ${WRAP_ENUM_DEST_IMPL}
            --with-includes ${WRAP_ENUM_WITH_INCLUDES}
            COMMENT "Generating enum files from ${WRAP_ENUM_SOURCE}"
            DEPENDS ${WRAP_ENUM_SOURCE}
            VERBATIM
    )
endfunction()
