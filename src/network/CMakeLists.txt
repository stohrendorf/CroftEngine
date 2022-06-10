include( get_boost )

add_library( haunted-coop STATIC hauntedcoopclient.cpp )
target_link_libraries(
        haunted-coop
        PRIVATE
        Boost::headers
        Boost::log
        gsl-lite::gsl-lite
        launcher
)
