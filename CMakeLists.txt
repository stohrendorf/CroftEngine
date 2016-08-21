# Usage (Linux):
#   $ cd OpenTombSrc
#   $ mkdir build && cd build
#   $ cmake .. && make

cmake_minimum_required(VERSION 3.2)

include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/stohrendorf/hunter/archive/f730e3c314f4568bf5f20850e6a5451dc691b318.zip"
    SHA1 "6dac4843726a6d8f6110f63da5af1dedb1931589"
)

project(OpenTomb)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

if(WIN32 OR APPLE)
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." ON)
else()
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." OFF)
endif()

message(STATUS "Always ignore system-provided libraries: ${DEFAULT_EXTERNAL_HUNTER}")


function(GROUP_FILES)
    foreach(fn ${ARGV})
        get_filename_component(parentDir "${fn}" PATH)

        # change /'s to \\'s, and reduce redundant // or \\.
        string(REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}")

        source_group("${grp}" FILES "${fn}")
    endforeach()
endfunction()

# alternative system-provided package name
macro(ot_find_dependency_alt depname sysdepname)
    option(SKIP_SYSTEM_${depname} "Skip the sytem-provided ${depname} installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
    if(SKIP_SYSTEM_${depname})
        hunter_add_package(${depname} ${ARGN})
        find_package(${depname} REQUIRED ${ARGN})
    else()
        # temporarily disable Hunter-local Find* modules
        set(_old_cmp ${CMAKE_MODULE_PATH})
        set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
        find_package(${sysdepname} MODULE REQUIRED ${ARGN})
        # restore module path
        set(CMAKE_MODULE_PATH ${_old_cmp})
        set(_old_cmp)
    endif()
    if(NOT TARGET ${depname})
        add_library(${depname} INTERFACE)
    endif()
endmacro()

macro(ot_find_dependency depname)
    ot_find_dependency_alt(${depname} ${depname} ${ARGN})
endmacro()


# Dependency: OpenAL
ot_find_dependency(OpenAL)
target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})

option(USE_OPENAL_FLOAT "Use OpenAL Soft floating-point extensions (not generally available)." OFF)
if(USE_OPENAL_FLOAT)
    target_compile_definitions(OpenAL INTERFACE -DAUDIO_OPENAL_FLOAT)
endif(USE_OPENAL_FLOAT)


# Dependency: OpenGL
find_package(OpenGL REQUIRED)
add_library(OpenGL INTERFACE)
target_include_directories(OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})
target_link_libraries(OpenGL INTERFACE ${OPENGL_LIBRARIES})


# Dependency: Lua
ot_find_dependency(Lua)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})



# Dependency: Boost
ot_find_dependency(Boost COMPONENTS system log filesystem thread)
if(NOT SKIP_SYSTEM_Boost)
    target_compile_definitions(Boost INTERFACE -DBOOST_LOG_DYN_LINK=0)
endif()
target_compile_definitions(Boost INTERFACE -DBOOST_ALL_NO_LIB)
target_include_directories(Boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost INTERFACE ${Boost_LIBRARIES})


# Dependency: ZLIB
ot_find_dependency(ZLIB)
target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARIES})
target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIR})


# Dependency: SndFile
ot_find_dependency(OGG)
ot_find_dependency(Vorbis) # requires OGG
ot_find_dependency(SndFile) # requires OGG and Vorbis
target_include_directories(SndFile INTERFACE ${SNDFILE_INCLUDE_DIR})
target_link_libraries(SndFile INTERFACE ${SNDFILE_LIBRARIES} ${OGG_LIBRARIES} ${VORBIS_LIBRARIES} ${VORBISFILE_LIBRARIES})


# Dependency: Freetype
ot_find_dependency_alt(freetype Freetype)
target_include_directories(freetype INTERFACE ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(freetype INTERFACE ${FREETYPE_LIBRARIES})

find_package(GLEW REQUIRED)
add_library(GLEW INTERFACE)
target_include_directories(GLEW INTERFACE ${GLEW_INCLUDE_DIRS})
target_link_libraries(GLEW INTERFACE ${GLEW_LIBRARIES})

if(MSVC)
    # C4503: Name too long
    add_definitions(/wd4503)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(Git)
if(NOT GIT_FOUND)
    set(GIT_SHA "<unknown>")
else()
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --dirty --tags --long
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()

add_definitions(-DGIT_SHA="${GIT_SHA}")

add_subdirectory(3rdparty/gameplay)
add_subdirectory(doc)
add_subdirectory(src)
