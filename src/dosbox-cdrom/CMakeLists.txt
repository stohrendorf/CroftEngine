add_library(
        dosbox-cdrom
        STATIC
        cdrom_image.cpp
        mscdex.cpp
        cueparser.cpp
        binaryfile.cpp
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