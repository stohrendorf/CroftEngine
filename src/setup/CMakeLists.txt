include( get_qt )
include( get_boost )
include( get_valvefilevdf )

set(
        SETUP_SCREEN_SRCS
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        setup.cpp
        downloadprogress.h
        downloadprogress.cpp
        downloadprogress.ui
)

set_property(
        SOURCE mainwindow.cpp
        PROPERTY COMPILE_DEFINITIONS CE_VERSION="${CMAKE_PROJECT_VERSION}"
)

set( TS_FILES )
file( STRINGS ../languages.txt _languages )
foreach( _language IN LISTS _languages )
    if( NOT _language MATCHES ".+ .+" )
        continue()
    endif()

    string( REGEX REPLACE " .+\$" "" _lang_id "${_language}" )
    if( _lang_id STREQUAL "en_GB" )
        continue()
    endif()
    list( APPEND TS_FILES "croftengine_${_lang_id}.ts" )
endforeach()

set_source_files_properties( ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_SOURCE_DIR}/share/i18n" )
qt5_create_translation( QM_FILES ${SETUP_SCREEN_SRCS} ${TS_FILES} )
# qt5_add_translation( QM_FILES ${TS_FILES} )

add_library( setup_screen STATIC ${SETUP_SCREEN_SRCS} ${QM_FILES} )

target_link_libraries(
        setup_screen
        PRIVATE
        Qt5::Widgets
        Qt5::Network
        shared
        Boost::headers
        Boost::log
        dosbox-cdrom
        dosbox-cdrom
        LibArchive::LibArchive
        valve-file-vdf::valve-file-vdf
        ryml
)
