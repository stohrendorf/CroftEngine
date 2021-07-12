if( VCPKG_TOOLCHAIN )
    include( dl_unpack )
    dl_unpack(
            URL https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-gettext-0.19.8.1-10-any.pkg.tar.zst
            FILENAME mingw-w64-x86_64-gettext-0.19.8.1-10-any.pkg.tar.zst
            TEST mingw64/bin/msgmerge.exe
            WORKING_DIRECTORY gettext
    )
    dl_unpack(
            URL https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libiconv-1.16-2-any.pkg.tar.zst
            FILENAME mingw-w64-x86_64-libiconv-1.16-2-any.pkg.tar.zst
            TEST mingw64/bin/libiconv-2.dll
            WORKING_DIRECTORY gettext
    )
    dl_unpack(
            URL https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-libwinpthread-git-9.0.0.6246.ae63cde27-1-any.pkg.tar.zst
            FILENAME mingw-w64-x86_64-libwinpthread-git-9.0.0.6246.ae63cde27-1-any.pkg.tar.zst
            TEST mingw64/bin/libwinpthread-1.dll
            WORKING_DIRECTORY gettext
    )
    dl_unpack(
            URL https://repo.msys2.org/mingw/mingw64/mingw-w64-x86_64-iconv-1.16-2-any.pkg.tar.zst
            FILENAME mingw-w64-x86_64-iconv-1.16-2-any.pkg.tar.zst
            TEST mingw64/bin/libintl-8.dll
            WORKING_DIRECTORY gettext
    )
    dl_unpack(
            URL https://mirror.msys2.org/mingw/mingw64/mingw-w64-x86_64-gcc-libs-10.3.0-5-any.pkg.tar.zst
            FILENAME mingw-w64-x86_64-gcc-libs-10.3.0-5-any.pkg.tar.zst
            TEST mingw64/bin/libstdc++-6.dll
            WORKING_DIRECTORY gettext
    )
    find_program(
            GETTEXT_MSGFMT_EXECUTABLE msgfmt
            PATHS ${EXTERNAL_SRC_ROOT}/gettext/mingw64/bin
    )
    if( NOT GETTEXT_MSGFMT_EXECUTABLE )
        message( FATAL_ERROR "Could not find msgfmt" )
    endif()
    find_program(
            GETTEXT_MSGMERGE_EXECUTABLE msgmerge
            PATHS ${EXTERNAL_SRC_ROOT}/gettext/mingw64/bin
    )
    if( NOT GETTEXT_MSGMERGE_EXECUTABLE )
        message( FATAL_ERROR "Could not find msgmerge" )
    endif()

    find_program(
            GETTEXT_XGETTEXT_EXECUTABLE xgettext
            PATHS ${EXTERNAL_SRC_ROOT}/gettext/mingw64/bin
    )
    find_program(
            GETTEXT_MSGINIT_EXECUTABLE msginit
            PATHS ${EXTERNAL_SRC_ROOT}/gettext/mingw64/bin
    )
else()
    find_package( Gettext REQUIRED )

    find_program(
            GETTEXT_XGETTEXT_EXECUTABLE xgettext
    )
    find_program(
            GETTEXT_MSGINIT_EXECUTABLE msginit
    )
endif()

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
        set( _gmo "${GETTEXT_POTFILE_DESTINATION}/po/${lang}/LC_MESSAGES/${GETTEXT_DOMAIN}.mo" )
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
                DEPENDS "${_pot}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMENT "Updating the ${lang} .po file from the .pot file" )

        add_custom_command(
                OUTPUT "${_gmo}"
                COMMAND "${GETTEXT_MSGFMT_EXECUTABLE}"
                "${_po}"
                "--output-file=${_gmo}"
                "--statistics"
                DEPENDS "${_po}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                COMMENT "Creating the ${lang} .gmo file from the .po file" )

        add_custom_target(
                "${GETTEXT_TARGET_NAME}-${lang}"
                DEPENDS "${_gmo}"
        )
        add_dependencies( "${GETTEXT_TARGET_NAME}" "${GETTEXT_TARGET_NAME}-${lang}" )
    endforeach()
endfunction()
