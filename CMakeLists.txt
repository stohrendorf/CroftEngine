cmake_minimum_required( VERSION 3.16 )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

project( EdisonEngine )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

if( MSVC AND CMAKE_CXX_CLANG_TIDY )
    set( CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};--extra-arg-before=-std=c++${CMAKE_CXX_STANDARD};--extra-arg-before=-EHsc" )
endif()
if( CMAKE_CXX_CPPCHECK )
    set( CMAKE_CXX_CPPCHECK "${CMAKE_CXX_CPPCHECK};--enable=warning,performance,portability,style;--language=c++;--std=c++${CMAKE_CXX_STANDARD};--inline-suppr" )
endif()

if( MSVC )
    # C4503: Name too long
    # C4996: "Call to 'std::copy_n' with parameters that may be unsafe" etc...
    add_definitions( -D_CRT_SECURE_NO_WARNINGS )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4503 /wd4996 /wd4251 /wd4275 /experimental:external /external:anglebrackets /MP" )
    string( REPLACE "/Ob0" "/Ob1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
endif()

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    message( STATUS "GCC or Clang detected" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fvisibility=hidden" )
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

function( group_files )
    foreach( fn ${ARGV} )
        get_filename_component( parentDir "${fn}" PATH )

        # change /'s to \\'s, and reduce redundant // or \\.
        string( REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}" )

        source_group( "${grp}" FILES "${fn}" )
    endforeach()
endfunction()

if( VCPKG_TOOLCHAIN )
    find_package( OpenAL CONFIG REQUIRED )
else()
    find_package( OpenAL REQUIRED )
    if( NOT TARGET OpenAL::OpenAL )
        add_library( OpenAL::OpenAL INTERFACE IMPORTED )
        target_include_directories( OpenAL::OpenAL SYSTEM BEFORE INTERFACE ${OPENAL_INCLUDE_DIR} )
        target_link_libraries( OpenAL::OpenAL INTERFACE ${OPENAL_LIBRARY} )
    endif()
endif()

find_package( OpenGL REQUIRED )
add_library( OpenGL INTERFACE )
target_include_directories( OpenGL SYSTEM BEFORE INTERFACE ${OPENGL_INCLUDE_DIR} )
target_link_libraries( OpenGL INTERFACE ${OPENGL_LIBRARIES} )

cmake_policy( SET CMP0094 NEW )

if( VCPKG_TOOLCHAIN )
    find_package( glm CONFIG REQUIRED )
else()
    find_package( glm REQUIRED )
    if( NOT TARGET glm )
        add_library( glm INTERFACE IMPORTED )
        target_include_directories( glm SYSTEM BEFORE INTERFACE ${GLM_INCLUDE_DIR} )
    endif()
endif()
target_compile_definitions( glm INTERFACE -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS )

if( MSVC )
    # https://github.com/boostorg/system/issues/32
    add_definitions( -DHAVE_SNPRINTF )
endif()

find_package( Boost COMPONENTS system log locale REQUIRED )
target_compile_definitions( Boost::boost INTERFACE -DBOOST_LOG_DYN_LINK=0 -DBOOST_ALL_NO_LIB )
target_include_directories( Boost::boost SYSTEM BEFORE INTERFACE ${Boost_INCLUDE_DIR} )
target_link_libraries( Boost::boost INTERFACE ${Boost_LIBRARIES} )

find_package( ZLIB REQUIRED )

if( VCPKG_TOOLCHAIN )
    find_package( SndFile REQUIRED )
else()
    find_path( LIBSNDFILE_INCLUDE_DIRS sndfile.h )
    find_library( LIBSNDFILE_LIBRARIES sndfile )

    if( NOT LIBSNDFILE_INCLUDE_DIRS OR NOT LIBSNDFILE_LIBRARIES )
        message( FATAL_ERROR "Could not find libsndfile" )
    endif()

    add_library( SndFile::sndfile INTERFACE IMPORTED )
    target_include_directories( SndFile::sndfile SYSTEM BEFORE INTERFACE "${LIBSNDFILE_INCLUDE_DIRS}" )
    target_link_libraries( SndFile::sndfile INTERFACE "${LIBSNDFILE_LIBRARIES}" )
endif()

find_package( FFmpeg REQUIRED )
add_library( FFmpeg INTERFACE )
target_include_directories( FFmpeg SYSTEM BEFORE INTERFACE ${FFMPEG_INCLUDE_DIR} )
target_link_libraries( FFmpeg INTERFACE ${FFMPEG_LIBRARY} )

include( cmake/dl_unpack.cmake )

if( VCPKG_TOOLCHAIN )
    find_package( CImg REQUIRED )
else()
    dl_unpack( https://framagit.org/dtschump/CImg/-/archive/v.2.8.0/CImg-v.2.8.0.tar.bz2 CImg-v.2.8.0.tar.bz2 CImg-v.2.8.0 )

    add_library( CImg::CImg INTERFACE IMPORTED )
    target_include_directories( CImg::CImg SYSTEM BEFORE INTERFACE "${CMAKE_SOURCE_DIR}/3rdparty/CImg-v.2.8.0" )
endif()
target_compile_definitions( CImg::CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png )

find_package( PNG REQUIRED )
target_link_libraries( CImg::CImg INTERFACE PNG::PNG )

find_package( Python3 COMPONENTS Development REQUIRED )

dl_unpack( https://github.com/pybind/pybind11/archive/v2.6.0.tar.gz pybind11-v2.6.0.tar.gz pybind11-2.6.0 )
set( PYBIND11_FINDPYTHON FALSE )
set( PYBIND11_NOPYTHON TRUE )
add_subdirectory( 3rdparty/pybind11-2.6.0 )

download( https://raw.githubusercontent.com/martinmoene/gsl-lite/4b796627ad0fa42640f5fdb96f23c4a0d9ee084f/include/gsl/gsl-lite.hpp gsl-lite/gsl-lite.hpp )

add_library( gsl-lite::gsl-lite INTERFACE IMPORTED )
target_include_directories( gsl-lite::gsl-lite SYSTEM BEFORE INTERFACE "${CMAKE_SOURCE_DIR}/3rdparty/gsl-lite" )
target_compile_definitions(
        gsl-lite::gsl-lite INTERFACE
        -Dgsl_FEATURE_WITH_CONTAINER_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_MAKE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_BYTE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_CONFIG_NOT_NULL_EXPLICIT_CTOR=0
        -Dgsl_CONFIG_CONTRACT_VIOLATION_CALLS_HANDLER
)

add_definitions( -DNOMINMAX -DNOGDI )

find_package( Qt5 COMPONENTS Widgets CONFIG REQUIRED )
add_definitions( -DQT_NO_KEYWORDS )

find_path(
        UTF8CPP_INCLUDE_DIR
        NAMES utf8.h
        PATH_SUFFIXES utf8cpp
)
if( NOT UTF8CPP_INCLUDE_DIR )
    message( FATAL_ERROR "utf8cpp not found" )
endif()
add_library( utf8cpp::utf8cpp INTERFACE IMPORTED )
target_include_directories( utf8cpp::utf8cpp INTERFACE "${UTF8CPP_INCLUDE_DIR}" )

find_package( ryml CONFIG REQUIRED )

add_subdirectory( 3rdparty/type_safe )
add_subdirectory( src )
