find_package( Gettext REQUIRED )

find_program(
        GETTEXT_XGETTEXT_EXECUTABLE xgettext
)
find_program(
        GETTEXT_MSGINIT_EXECUTABLE msginit
)

if( NOT GETTEXT_XGETTEXT_EXECUTABLE OR NOT GETTEXT_MSGINIT_EXECUTABLE )
    message( FATAL_ERROR "Could not find required programs!" )
endif()

find_package( Intl REQUIRED )

# configure_gettext(
#     DOMAIN <domain-name>
#     TARGET_NAME <target-name>
#     SOURCES <file> ...
#     POTFILE_DESTINATION <dir>
#     LANGUAGES <file> ...
#     [XGETTEXT_ARGS <args> ...
#     )

function( configure_gettext )
    set( one_value_args DOMAIN TARGET_NAME POTFILE_DESTINATION )
    set( multi_args SOURCES LANGUAGES XGETTEXT_ARGS )
    cmake_parse_arguments(
            GETTEXT
            ""
            "${one_value_args}"
            "${multi_args}"
            ${ARGV}
    )

    if( NOT GETTEXT_DOMAIN )
        message( FATAL_ERROR "Must supply a DOMAIN!" )
    endif()
    if( NOT GETTEXT_POTFILE_DESTINATION )
        message( FATAL_ERROR "Must supply a POTFILE_DESTINATION!" )
    endif()
    if( NOT GETTEXT_LANGUAGES )
        message( FATAL_ERROR "No LANGUAGES specified!" )
    endif()
    if( NOT GETTEXT_TARGET_NAME )
        message( FATAL_ERROR "No TARGET_NAME specified!" )
    endif()
    if( NOT GETTEXT_SOURCES )
        message( FATAL_ERROR "No SOURCES supplied!" )
    endif()

    # Make input directories absolute in relation to the current directory
    if( NOT IS_ABSOLUTE "${GETTEXT_POTFILE_DESTINATION}" )
        set( GETTEXT_POTFILE_DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/${GETTEXT_POTFILE_DESTINATION}" )
        file( TO_CMAKE_PATH "${GETTEXT_POTFILE_DESTINATION}" GETTEXT_POTFILE_DESTINATION )
    endif()

    # Create needed directories
    if( NOT EXISTS "${GETTEXT_POTFILE_DESTINATION}" )
        file( MAKE_DIRECTORY "${GETTEXT_POTFILE_DESTINATION}" )
    endif()

    add_custom_target( "${GETTEXT_TARGET_NAME}" )

    # Generate the .pot file from the program sources
    # sources ---{xgettext}---> .pot
    set( _pot "${GETTEXT_POTFILE_DESTINATION}/${GETTEXT_DOMAIN}.pot" )
    add_custom_command(
            TARGET "${GETTEXT_TARGET_NAME}"
            COMMAND "${GETTEXT_XGETTEXT_EXECUTABLE}" ${GETTEXT_XGETTEXT_ARGS}
            ${GETTEXT_SOURCES}
            "--output=${_pot}"
            "--boost"
            "--add-comment=translators:"
            "--language=C++"
            "--from-code=UTF-8"
            "--sort-output"
            "--escape"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Generating ${_pot}"
    )

    foreach( lang IN LISTS GETTEXT_LANGUAGES )
        # Create needed directories
        if( NOT EXISTS "${GETTEXT_POTFILE_DESTINATION}/po/${lang}/LC_MESSAGES" )
            file( MAKE_DIRECTORY "${GETTEXT_POTFILE_DESTINATION}/po/${lang}/LC_MESSAGES" )
        endif()

        # .pot ---{msginit}---> .po
        set( _po "${GETTEXT_POTFILE_DESTINATION}/po/${lang}/${GETTEXT_DOMAIN}.po" )
        set( _mo "${GETTEXT_POTFILE_DESTINATION}/po/${lang}/LC_MESSAGES/${GETTEXT_DOMAIN}.mo" )
        if( NOT EXISTS "${_po}" )
            message( STATUS "Creating initial .po file for ${lang}" )
            execute_process(
                    COMMAND "${GETTEXT_MSGINIT_EXECUTABLE}"
                    "--input=${_pot}"
                    "--output-file=${_po}"
                    "--locale=${lang}"
                    "--no-translator"
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" )
        endif()
        add_custom_command(
                OUTPUT "${_po}"
                COMMAND "${GETTEXT_MSGMERGE_EXECUTABLE}"
                "${_po}"
                "${_pot}"
                "--output-file=${_po}"
                "--escape"
                DEPENDS "${_pot}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMENT "Updating the ${lang} .po file from the .pot file" )

        add_custom_command(
                OUTPUT "${_mo}"
                COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}"
                "${_po}"
                "--output-file=${_mo}"
                "--statistics"
                DEPENDS "${_po}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMENT "Creating the ${lang} .mo file from the .po file" )

        add_custom_target(
                "${GETTEXT_TARGET_NAME}-${lang}"
                DEPENDS "${_mo}"
        )
        add_dependencies( "${GETTEXT_TARGET_NAME}" "${GETTEXT_TARGET_NAME}-${lang}" )
    endforeach()
endfunction()
