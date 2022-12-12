add_library(
        dosbox-cdrom
        STATIC
        discimage.cpp
        discfs.cpp
        cueparser.cpp
        binaryfile.cpp
        physicaltrackbuilder.cpp
)

target_link_libraries(
        dosbox-cdrom
        PRIVATE
        Boost::log
        gsl-lite::gsl-lite
)

target_include_directories(
        dosbox-cdrom
        PUBLIC
        .
)