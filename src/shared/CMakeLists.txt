include( get_boost )

add_library(
        shared
        STATIC
        stacktrace.cpp
)

target_include_directories( shared PUBLIC . )

target_link_libraries( shared PRIVATE Boost::stacktrace Boost::log )
