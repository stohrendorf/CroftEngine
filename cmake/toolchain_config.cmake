set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

enable_testing()

if( CMAKE_BUILD_TYPE STREQUAL "Release" )
    include( CheckIPOSupported )
    check_ipo_supported()
    set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ON )
endif()

if( MSVC AND CMAKE_CXX_CLANG_TIDY )
    set( CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--extra-arg-before=-std=c++${CMAKE_CXX_STANDARD};--extra-arg-before=-EHsc" )
endif()
if( CMAKE_CXX_CPPCHECK )
    set( CMAKE_CXX_CPPCHECK "${CMAKE_CXX_CPPCHECK};--enable=warning,performance,portability,style;--language=c++;--std=c++${CMAKE_CXX_STANDARD};--inline-suppr" )
endif()

if( MSVC )
    include( dl_unpack )
    add_definitions( -D_CRT_SECURE_NO_WARNINGS )
    # C4201: anonymous structs/unions
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP /wd4201 /experimental:external /external:I ${EXTERNAL_SRC_ROOT}" )
    string( REPLACE "/Ob0" "/Ob1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1" )
endif()

if( WIN32 )
    add_definitions( -DNOMINMAX -DNOGDI -DNOBITMAP )
endif()

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    message( STATUS "GCC or Clang detected" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wdouble-promotion -Wstrict-overflow=2 -Wparentheses -fvisibility=hidden" )
    if( NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=cold -Wsuggest-attribute=noreturn -Wduplicated-cond -Wtrampolines -Wunsafe-loop-optimizations -Wcast-align=strict -Wparentheses -Wlogical-op" )
    else()
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align" )
    endif()

    if( MINGW )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj" )
    endif()

    option( SANITIZE_ADDRESS "Use -fsanitize=address" OFF )
    if( SANITIZE_ADDRESS )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" )
    endif()

    option( SANITIZE_UNDEFINED "Use -fsanitize=undefined" OFF )
    if( SANITIZE_UNDEFINED )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined" )
    endif()
endif()
