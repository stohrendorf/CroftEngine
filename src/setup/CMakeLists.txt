include( get_qt )
include( get_boost )
include( get_libarchive )

add_library(
        setup_screen
        STATIC
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
        PROPERTY COMPILE_DEFINITIONS EE_VERSION="${CMAKE_PROJECT_VERSION}"
)

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
)
