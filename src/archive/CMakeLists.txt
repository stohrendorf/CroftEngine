include( get_libarchive )
include( get_gsllite )
include( get_boost )

add_library(
        archive
        STATIC
        readonlyarchive.h
        readonlyarchive.cpp
        writeonlyxzarchive.h
        writeonlyxzarchive.cpp
)

target_link_libraries(
        archive
        PUBLIC
        gsl-lite::gsl-lite
        Boost::log
        Boost::log_setup
        Boost::disable_autolinking
)

target_link_libraries(
        archive
        PRIVATE
        LibArchive::LibArchive
        Boost::headers
)

target_include_directories(
        archive
        PUBLIC
        .
)