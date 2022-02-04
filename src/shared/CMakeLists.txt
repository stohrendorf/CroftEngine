include( get_boost )

add_library(
        shared
        STATIC
        stacktrace.cpp
        paths.cpp
)

target_compile_definitions(
        shared
        PRIVATE
        -DEE_DATAROOT_DIR=${CMAKE_INSTALL_FULL_DATAROOTDIR}/edisonengine
)

target_include_directories(
        shared
        PUBLIC
        .
)

target_link_libraries(
        shared
        PRIVATE
        Boost::stacktrace
        Boost::log
)

target_link_libraries(
        shared
        PUBLIC
        gsl-lite::gsl-lite
)
