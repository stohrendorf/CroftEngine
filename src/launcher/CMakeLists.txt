include( get_qt )
include( get_boost )
include( get_valvefilevdf )

set(
        LAUNCHER_SRCS
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        launcher.cpp
        downloadprogress.h
        downloadprogress.cpp
        downloadprogress.ui
        networkconfig.h
        networkconfig.cpp
)

set_property(
        SOURCE mainwindow.cpp
        PROPERTY COMPILE_DEFINITIONS "CE_VERSION=\"${CMAKE_PROJECT_VERSION}\";CE_GIT_TAG=\"${CE_GIT_TAG}\""
)

set( TS_FILES )
file( STRINGS ../languages.txt _languages )
set( _lang_list_tmp ${CMAKE_BINARY_DIR}/lang-list.tmp )
file( APPEND "${_lang_list_tmp}" "#pragma once\n" )
file( APPEND "${_lang_list_tmp}" "#include <vector>\n" )
file( APPEND "${_lang_list_tmp}" "#include <tuple>\n" )
file( APPEND "${_lang_list_tmp}" "#include <string>\n" )
file( APPEND "${_lang_list_tmp}" "inline std::vector<std::tuple<std::string, std::string>> getSupportedLanguages()\n" )
file( APPEND "${_lang_list_tmp}" "{\n" )
file( APPEND "${_lang_list_tmp}" "    return {\n" )
foreach( _language IN LISTS _languages )
    if( NOT _language MATCHES ".+ .+" )
        continue()
    endif()

    string( REGEX REPLACE " .+\$" "" _lang_id "${_language}" )
    string( REGEX REPLACE "^[^ ]+ " "" _lang_name "${_language}" )

    file( APPEND "${_lang_list_tmp}" "        {\"${_lang_id}\", \"${_lang_name}\"},\n" )

    if( _lang_id STREQUAL "en_GB" )
        continue()
    endif()
    list( APPEND TS_FILES "croftengine_${_lang_id}.ts" )
endforeach()
file( APPEND "${_lang_list_tmp}" "    };\n" )
file( APPEND "${_lang_list_tmp}" "}\n" )
file( RENAME ${_lang_list_tmp} ${CMAKE_CURRENT_SOURCE_DIR}/languages.h )

set_source_files_properties( ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_SOURCE_DIR}/share/i18n" )
qt5_create_translation( QM_FILES ${LAUNCHER_SRCS} ${TS_FILES} )
qt5_add_translation( QM_FILES ${TS_FILES} )

add_library( launcher STATIC ${LAUNCHER_SRCS} ${QM_FILES} )

target_link_libraries(
        launcher
        PRIVATE
        Qt5::Widgets
        Qt5::Network
        shared
        Boost::headers
        Boost::log
        dosbox-cdrom
        archive
        valve-file-vdf::valve-file-vdf
        ryml
        gameflow
        archive
        serialization
)
