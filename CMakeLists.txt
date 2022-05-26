cmake_minimum_required( VERSION 3.16 )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

project( CroftEngine VERSION 1.4.1 )

cmake_policy( SET CMP0069 NEW )
cmake_policy( SET CMP0094 NEW )

include( toolchain_config )

function( group_files )
    foreach( fn ${ARGV} )
        get_filename_component( parentDir "${fn}" PATH )

        # change /'s to \\'s, and reduce redundant // or \\.
        string( REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}" )

        source_group( "${grp}" FILES "${fn}" )
    endforeach()
endfunction()

include( GNUInstallDirs )
if( WIN32 )
    set( CMAKE_INSTALL_BINDIR "." )
endif()

include( get_opengl )
include( get_glm )
include( get_boost )
include( get_ffmpeg )
include( get_cimg )
include( dl_unpack )
include( get_python3 )
include( get_pybind11 )
include( get_gsllite )
include( get_openal )
include( get_utf8cpp )
include( get_gettext )
include( get_libarchive )

add_subdirectory( 3rdparty/rapidyaml EXCLUDE_FROM_ALL )
add_subdirectory( 3rdparty/type_safe EXCLUDE_FROM_ALL )
add_subdirectory( src )

if( UNIX )
    install(
            FILES share/logo_256.png
            DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/pixmaps
            RENAME croftengine.png
    )
    install(
            FILES share/croftengine.desktop
            DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications
    )
endif()

if( WIN32 )
    add_custom_target(
            prepare-release
            COMMENT Building release dependencies...
            DEPENDS croftengine croftengine-python3-deps croftengine-runtime-deps
    )

    set( CPACK_GENERATOR NSIS 7Z )
    set( CPACK_BINARY_7Z ON )
    set( CPACK_BINARY_NSIS ON )
    set( CPACK_NSIS_COMPRESSOR "/SOLID lzma" )

    set( CPACK_PACKAGE_NAME CroftEngine )
    set( CPACK_PACKAGE_INSTALL_DIRECTORY CroftEngine )
    set( CPACK_PACKAGE_VENDOR stohrendorf )
    set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "An open-source Tomb Raider 1 engine remake" )
    set( CPACK_PACKAGE_HOMEPAGE_URL https://github.com/stohrendorf/CroftEngine/ )
    set( CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_HOMEPAGE_URL} )
    set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/share/LICENSE )
    set( CPACK_MONOLITHIC_INSTALL ON )
    set( CPACK_PACKAGE_EXECUTABLES "croftengine;CroftEngine" )
    set( CPACK_NSIS_EXTRA_INSTALL_COMMANDS
            "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\CroftEngine Configuration.lnk\\\" \\\"$INSTDIR\\\\.\\\\croftengine.exe\\\" \\\"--configure\\\""
            "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\CroftEngine (English).lnk\\\" \\\"$INSTDIR\\\\.\\\\croftengine.exe\\\" \\\"--locale en_GB.utf8\\\""
            "CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\CroftEngine (Deutsch).lnk\\\" \\\"$INSTDIR\\\\.\\\\croftengine.exe\\\" \\\"--locale de_DE.utf8\\\""
            )
    set( CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
            "Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\CroftEngine Configuration.lnk\\\""
            "Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\CroftEngine (English).lnk\\\""
            "Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\CroftEngine (Deutsch).lnk\\\""
            )

    string( REPLACE ";" "\n" CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}" )
    string( REPLACE ";" "\n" CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}" )

    set( CPACK_SOURCE_GENERATOR "7Z" )
    set( CPACK_NSIS_EXECUTABLES_DIRECTORY . )
    set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )

    include( CPack )
endif()