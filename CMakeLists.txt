cmake_minimum_required( VERSION 3.11.2 )
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake )

project( EdisonEngine )

set( CMAKE_CXX_STANDARD 14 )

if( MSVC )
    # C4503: Name too long
    # C4996: "Call to 'std::copy_n' with parameters that may be unsafe" etc...
    add_definitions( -D_CRT_SECURE_NO_WARNINGS )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4503 /wd4996 /wd4251 /wd4275 /MP" )
    string( REPLACE "/Ob0" "/Ob1" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" )
endif()

if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
    message( STATUS "GCC or Clang detected" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra" )
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
        target_include_directories( OpenAL::OpenAL INTERFACE ${OPENAL_INCLUDE_DIR} )
        target_link_libraries( OpenAL::OpenAL INTERFACE ${OPENAL_LIBRARY} )
    endif()
endif()

find_package( OpenGL REQUIRED )
add_library( OpenGL INTERFACE )
target_include_directories( OpenGL INTERFACE ${OPENGL_INCLUDE_DIR} )
target_link_libraries( OpenGL INTERFACE ${OPENGL_LIBRARIES} )

find_path( LUA_INCLUDE_DIR lua.hpp PATH_SUFFIXES lua5.3 )
find_library( LUA_LIBRARIES lua )

if( NOT LUA_LIBRARIES OR NOT LUA_INCLUDE_DIR )
    message( FATAL_ERROR "Lua not found" )
endif()

add_library( Lua INTERFACE )
target_link_libraries( Lua INTERFACE ${LUA_LIBRARIES} )
target_include_directories( Lua INTERFACE ${LUA_INCLUDE_DIR} )

if( VCPKG_TOOLCHAIN )
    find_package( glm CONFIG REQUIRED )
else()
    find_package( glm REQUIRED )
    if( NOT TARGET glm )
        add_library( glm INTERFACE IMPORTED )
        target_include_directories( glm INTERFACE ${GLM_INCLUDE_DIR} )
    endif()
endif()
target_compile_definitions( glm INTERFACE -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS )

find_package( Boost COMPONENTS system log filesystem thread REQUIRED )
target_compile_definitions( Boost::boost INTERFACE -DBOOST_LOG_DYN_LINK=0 )
target_compile_definitions( Boost::boost INTERFACE -DBOOST_ALL_NO_LIB )
target_include_directories( Boost::boost INTERFACE ${Boost_INCLUDE_DIR} )
target_link_libraries( Boost::boost INTERFACE ${Boost_LIBRARIES} )

find_package( ZLIB REQUIRED )

find_package( PNG REQUIRED )

if( VCPKG_TOOLCHAIN )
    find_package( LibSndFile REQUIRED )
else()
    find_path( LIBSNDFILE_INCLUDE_DIRS sndfile.h )
    find_library( LIBSNDFILE_LIBRARIES sndfile )

    if( NOT LIBSNDFILE_INCLUDE_DIRS OR NOT LIBSNDFILE_LIBRARIES )
        message( FATAL_ERROR "Could not find libsndfile" )
    endif()

    add_library( sndfile-shared INTERFACE IMPORTED )
    target_include_directories( sndfile-shared INTERFACE "${LIBSNDFILE_INCLUDE_DIRS}" )
    target_link_libraries( sndfile-shared INTERFACE "${LIBSNDFILE_LIBRARIES}" )
endif()

find_package( Freetype REQUIRED )

find_package( GLEW REQUIRED )

find_package( glfw3 3.2 REQUIRED )

find_package( FFmpeg REQUIRED )
add_library( FFmpeg INTERFACE )
target_include_directories( FFmpeg INTERFACE ${FFMPEG_INCLUDE_DIR} )
target_link_libraries( FFmpeg INTERFACE ${FFMPEG_LIBRARY} )

if( VCPKG_TOOLCHAIN )
    find_package( CImg REQUIRED )
else()
    if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/CImg-v.2.3.6.zip" )
        message( STATUS "Downloading CImg..." )
        file( DOWNLOAD https://framagit.org/dtschump/CImg/-/archive/v.2.3.6/CImg-v.2.3.6.zip "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/CImg-v.2.3.6.zip"
              STATUS status
              LOG log )
        list( GET status 0 status_code )
        list( GET status 1 status_string )

        if( NOT status_code EQUAL 0 )
            message( FATAL_ERROR "Download failed with code ${status_code}" )
        endif()
    endif()

    if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/CImg-v.2.3.6" )
        message( STATUS "Extracting CImg..." )
        execute_process( COMMAND ${CMAKE_COMMAND} -E tar xfz CImg-v.2.3.6.zip
                         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty"
                         RESULT_VARIABLE rv )

        if( NOT rv EQUAL 0 )
            message( FATAL_ERROR "Failed to extract CImg-v.2.3.6.zip" )
        endif()
    endif()

    add_library( CImg::CImg INTERFACE IMPORTED )
    target_include_directories( CImg::CImg INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/CImg-v.2.3.6" )
endif()
target_compile_definitions( CImg::CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png )

if( VCPKG_TOOLCHAIN )
    find_package( sol2 CONFIG REQUIRED )
else()
    if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sol-v2.20.4/sol.hpp" )
        message( STATUS "Downloading sol2..." )
        file( DOWNLOAD https://github.com/ThePhD/sol2/releases/download/v2.20.4/sol.hpp "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sol-v2.20.4/sol.hpp"
              STATUS status
              LOG log )
        list( GET status 0 status_code )
        list( GET status 1 status_string )

        if( NOT status_code EQUAL 0 )
            message( FATAL_ERROR "Download failed with code ${status_code}" )
        endif()
    endif()

    add_library( sol2::sol2 INTERFACE IMPORTED )
    target_include_directories( sol2::sol2 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/sol-v2.20.4/" )
endif()

if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gsl-lite/gsl-lite.hpp" )
    message( STATUS "Downloading gsl-lite..." )
    file( DOWNLOAD https://raw.githubusercontent.com/martinmoene/gsl-lite/2871e5be5eb95117cf6b13f508cbd2f4a50e8801/include/gsl/gsl-lite.hpp "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gsl-lite/gsl-lite.hpp"
          STATUS status
          LOG log )
    list( GET status 0 status_code )
    list( GET status 1 status_string )

    if( NOT status_code EQUAL 0 )
        message( FATAL_ERROR "Download failed with code ${status_code}" )
    endif()
endif()

if( NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gamecontrollerdb.txt" )
    message( STATUS "Downloading gamecontrollerdb..." )
    file( DOWNLOAD https://raw.githubusercontent.com/gabomdq/SDL_GameControllerDB/master/gamecontrollerdb.txt "${CMAKE_CURRENT_SOURCE_DIR}/gamecontrollerdb.txt"
          STATUS status
          LOG log )
    list( GET status 0 status_code )
    list( GET status 1 status_string )

    if( NOT status_code EQUAL 0 )
        message( FATAL_ERROR "Download failed with code ${status_code}" )
    endif()
endif()

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/gsl-lite" )
add_definitions(
        -Dgsl_FEATURE_WITH_CONTAINER_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_MAKE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_FEATURE_BYTE_SPAN_TO_STD=${CMAKE_CXX_STANDARD}
        -Dgsl_CONFIG_NOT_NULL_EXPLICIT_CTOR=0
        -Dgsl_CONFIG_CONTRACT_VIOLATION_CALLS_HANDLER
)

find_package( Git REQUIRED )
execute_process(
        # COMMAND ${GIT_EXECUTABLE} describe --dirty --tags --long
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_definitions( -DGIT_SHA="${GIT_SHA}" )
add_definitions( -DNOMINMAX -DNOGDI )

find_package( yaml-cpp REQUIRED )

add_subdirectory( 3rdparty/type_safe )
add_subdirectory( src )
