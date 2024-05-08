set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

include( CheckPIESupported )
check_pie_supported( OUTPUT_VARIABLE _pie_error )
if( CMAKE_CXX_LINK_PIE_SUPPORTED )
    set( CMAKE_POSITION_INDEPENDENT_CODE ON )
elseif( NOT MSVC )
    message( WARNING "PIE is not supported: ${_pie_error}" )
endif()

option( PERMISSIVE_BUILD "Disable a few compiler flags which are not strictly relevant for releases" OFF )

enable_testing()

if( CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" )
    include( CheckIPOSupported )
    check_ipo_supported()
    set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ON )
endif()

if( MSVC AND CMAKE_CXX_CLANG_TIDY )
    set( CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--extra-arg-before=-std=c++${CMAKE_CXX_STANDARD};--extra-arg-before=-EHsc;--header-filter=.*" )
endif()
if( CMAKE_CXX_INCLUDE_WHAT_YOU_USE )
    set( CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${CMAKE_CXX_INCLUDE_WHAT_YOU_USE};-Xiwyu;--no_comments;-Xiwyu;--cxx17ns;-Xiwyu;--mapping_file=${CMAKE_SOURCE_DIR}/iwyu-mappings.yaml" )
endif()
if( CMAKE_CXX_CPPCHECK )
    file( MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/cppcheck" )
    if( WIN32 )
        set( _platform_defs "-D_WIN64;-D_MSC_VER=1930;-DC4_CPP=${CMAKE_CXX_STANDARD}" )
    else()
        set( _platform_defs "-D__linux__;-D_LIBCPP_ABI_NAMESPACE" )
    endif()
    set( CMAKE_CXX_CPPCHECK "${CMAKE_CXX_CPPCHECK};--enable=warning,performance,portability,style;--language=c++;--std=c++${CMAKE_CXX_STANDARD};--inline-suppr;--cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck;--suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck-suppressions.txt;--platform=native;${_platform_defs};-D__x86_64;-D_M_X64;-Dslots" )
    add_definitions( -D__cppcheck__ )
    set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

    add_custom_target(
            cppcheck-all
            COMMAND ${CMAKE_CXX_CPPCHECK}
            --enable=all
            --language=c++
            --std=c++${CMAKE_CXX_STANDARD}
            --inline-suppr
            --cppcheck-build-dir=${CMAKE_BINARY_DIR}/cppcheck
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
            --report-progress
            -j 8
            COMMENT "Running cppcheck"
    )
endif()

if( MSVC )
    # C4201: anonymous structs/unions
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MP /wd4201 /experimental:external /external:W0 /external:templates-" )

    string( REPLACE "/Ob0" "/Ob1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
    set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1" )
endif()

if( WIN32 )
    add_definitions( -DNOMINMAX -DNOGDI -DNOBITMAP -DNOMB )
endif()

option( SANITIZE_ADDRESS "Use -fsanitize=address" OFF )

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    message( STATUS "GCC or Clang detected" )

    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden" )

    if( MINGW )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj" )
    endif()

    if( SANITIZE_ADDRESS )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" )
    endif()

    option( SANITIZE_UNDEFINED "Use -fsanitize=undefined" OFF )
    if( SANITIZE_UNDEFINED )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined" )
    endif()

    if( NOT PERMISSIVE_BUILD )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wdouble-promotion -Wstrict-overflow=2 -Wparentheses" )
        if( NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=cold -Wsuggest-attribute=noreturn -Wduplicated-cond -Wtrampolines -Wunsafe-loop-optimizations -Wcast-align=strict -Wparentheses -Wlogical-op" )
        else()
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align" )
        endif()
    endif()
elseif( MSVC )
    if( SANITIZE_ADDRESS )
        get_filename_component( _linker_path "${CMAKE_LINKER}" DIRECTORY )
        file( GLOB _clang_rt "${_linker_path}/clang_rt.*.dll" )
        file( COPY ${_clang_rt} DESTINATION ${PROJECT_BINARY_DIR}/src )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address" )
    endif()
endif()
