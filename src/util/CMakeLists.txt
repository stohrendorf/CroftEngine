include( boost_test )
include( get_gettext )

add_boost_test( util_test
        tests/test_main.cpp
        tests/test_helpers.cpp
        tests/test_md5.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/md5.cpp
        ${CMAKE_SOURCE_DIR}/src/core/vec.cpp
        ${CMAKE_SOURCE_DIR}/src/core/angle.cpp
        ${CMAKE_SOURCE_DIR}/src/core/i18n.cpp
)

target_link_libraries( util_test PRIVATE serialization ${Intl_LIBRARIES} )
