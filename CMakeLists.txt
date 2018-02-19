cmake_minimum_required(VERSION 3.2)

include("cmake/HunterGate.cmake")

HunterGate(
        URL "https://github.com/stohrendorf/hunter/archive/c5f8b34b8c50a925c30434efc05eb88b4740aded.tar.gz"
        SHA1 "61fc48ea4bbb5a47ba3212d3ceaa2a8788f18e39"
)

project(EdisonEngine)

set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    # C4503: Name too long
    add_definitions(/wd4503)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/MP)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Ob1")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC or Clang detected")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

    option(SANITIZE_ADDRESS "Use -fsanitize=address" OFF)
    if (SANITIZE_ADDRESS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    endif ()

    option(SANITIZE_UNDEFINED "Use -fsanitize=undefined" OFF)
    if (SANITIZE_UNDEFINED)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
    endif ()
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

option(SKIP_NO_SYSTEM_LIBS "Override the default setting so that system libraries are never skipped when searching for dependencies" OFF)
if ((WIN32 OR APPLE) AND NOT SKIP_NO_SYSTEM_LIBS)
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." ON)
else ()
    option(DEFAULT_EXTERNAL_HUNTER "Always ignore system-provided libraries." OFF)
endif ()

message(STATUS "Always ignore system-provided libraries: ${DEFAULT_EXTERNAL_HUNTER}")


function(GROUP_FILES)
    foreach (fn ${ARGV})
        get_filename_component(parentDir "${fn}" PATH)

        # change /'s to \\'s, and reduce redundant // or \\.
        string(REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}")

        source_group("${grp}" FILES "${fn}")
    endforeach ()
endfunction()

# alternative system-provided package name
macro(ot_find_dependency_alt depname sysdepname)
    option(SKIP_SYSTEM_${depname} "Skip the sytem-provided ${depname} installation and use a custom built" ${DEFAULT_EXTERNAL_HUNTER})
    if (SKIP_SYSTEM_${depname})
        hunter_add_package(${depname} ${ARGN})
        find_package(${depname} REQUIRED ${ARGN})
    else ()
        # temporarily disable Hunter-local Find* modules
        set(_old_cmp ${CMAKE_MODULE_PATH})
        set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
        find_package(${sysdepname} MODULE REQUIRED ${ARGN})
        # restore module path
        set(CMAKE_MODULE_PATH ${_old_cmp})
        set(_old_cmp)
    endif ()
    if (NOT TARGET ${depname})
        add_library(${depname} INTERFACE)
    endif ()
endmacro()

macro(ot_find_dependency depname)
    ot_find_dependency_alt(${depname} ${depname} ${ARGN})
endmacro()


# Dependency: OpenAL
ot_find_dependency(OpenAL)
target_include_directories(OpenAL INTERFACE ${OPENAL_INCLUDE_DIR})
target_link_libraries(OpenAL INTERFACE ${OPENAL_LIBRARY})

option(USE_OPENAL_FLOAT "Use OpenAL Soft floating-point extensions (not generally available)." OFF)
if (USE_OPENAL_FLOAT)
    target_compile_definitions(OpenAL INTERFACE -DAUDIO_OPENAL_FLOAT)
endif (USE_OPENAL_FLOAT)


# Dependency: OpenGL
find_package(OpenGL REQUIRED)
add_library(OpenGL INTERFACE)
target_include_directories(OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})
target_link_libraries(OpenGL INTERFACE ${OPENGL_LIBRARIES})


# Dependency: Lua
ot_find_dependency(Lua)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})

if (LUA_VERSION_STRING VERSION_LESS "5.3")
    message(FATAL_ERROR "Your Lua version is ${LUA_VERSION_STRING}, but you need at least 5.3; either update your Lua installation, or set SKIP_SYSTEM_Lua=ON")
endif ()


ot_find_dependency_alt(glm GLM)
target_link_libraries(glm INTERFACE ${GLM_LIBRARIES})
target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})


# Dependency: Boost
ot_find_dependency(Boost COMPONENTS system log filesystem thread)
if (NOT SKIP_SYSTEM_Boost)
    target_compile_definitions(Boost INTERFACE -DBOOST_LOG_DYN_LINK=0)
endif ()
target_compile_definitions(Boost INTERFACE -DBOOST_ALL_NO_LIB)
target_include_directories(Boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost INTERFACE ${Boost_LIBRARIES})


# Dependency: ZLIB
ot_find_dependency(ZLIB)
target_link_libraries(ZLIB INTERFACE ${ZLIB_LIBRARIES})
target_include_directories(ZLIB INTERFACE ${ZLIB_INCLUDE_DIR})


# Dependency: PNG
ot_find_dependency(PNG)
target_link_libraries(PNG INTERFACE ${PNG_LIBRARIES} ZLIB)
target_include_directories(PNG INTERFACE ${PNG_INCLUDE_DIR})


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

ot_find_dependency_alt(glew GLEW)
add_library(GLEW INTERFACE)
target_include_directories(GLEW INTERFACE ${GLEW_INCLUDE_DIRS})
target_link_libraries(GLEW INTERFACE ${GLEW_LIBRARIES})

ot_find_dependency_alt(glfw GLFW)
add_library(GLFW INTERFACE)
target_include_directories(GLFW INTERFACE ${GLFW_INCLUDE_DIR})
target_link_libraries(GLFW INTERFACE ${GLFW_LIBRARIES})

add_library(CImg INTERFACE)
target_include_directories(CImg INTERFACE 3rdparty/CImg)
target_link_libraries(CImg INTERFACE PNG)
target_compile_definitions(CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png)

if (CYGWIN)
    find_package(X11 REQUIRED)
    target_link_libraries(GLFW INTERFACE ${X11_LIBRARIES})
    target_link_libraries(GLFW INTERFACE Xrandr Xinerama Xcursor)
endif ()


find_package(Git REQUIRED)
execute_process(
        # COMMAND ${GIT_EXECUTABLE} describe --dirty --tags --long
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_definitions(-DGIT_SHA="${GIT_SHA}")

include(ExternalProject)

ExternalProject_Add(
        ext-yamlcpp
        PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/yaml-cpp"
        INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/yaml-cpp-bin"
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG release-0.5.3
        EXCLUDE_FROM_ALL 1
        PATCH_COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/yaml-cpp-autoptr.diff
        CMAKE_ARGS
        -DBUILD_SHARED_LIBS=OFF
        -DBoost_DIR=${Boost_DIR}
        -DBoost_INCLUDE_DIR=${Boost_INCLUDE_DIR}
        -DBoost_LIBRARY_DIR=${Boost_LIBRARY_DIR}
        -DYAML_CPP_BUILD_TOOLS=OFF
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/yaml-cpp-bin
)

ExternalProject_Get_Property(ext-yamlcpp BINARY_DIR)
set(YAML_BUILDDIR "${BINARY_DIR}")
message(STATUS "YAML-CPP Build directory is ${YAML_BUILDDIR}")

add_library(yaml-cpp-lib INTERFACE)

if (EXISTS ${YAML_BUILDDIR}/yaml-cpp-config.cmake)
    include(${YAML_BUILDDIR}/yaml-cpp-config.cmake)
    if (NOT TARGET yaml-cpp)
        message(FATAL_ERROR "YAML-CPP is not configured correctly.")
    endif ()

    target_include_directories(yaml-cpp-lib INTERFACE ${YAML_CPP_INCLUDE_DIR})
    target_link_libraries(yaml-cpp-lib INTERFACE ${YAML_CPP_LIBRARIES})
else ()
    message(STATUS "***** YAML-CPP is not built yet. Build the ext-yamlcpp target, and then reconfigure")
endif ()


ExternalProject_Add(
        ext-assimp
        PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp"
        INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp-bin"
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v3.3.1
        EXCLUDE_FROM_ALL 1
        PATCH_COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp-cygwin-patch.diff
        CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp-bin
        -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
        -DASSIMP_BUILD_SAMPLES=OFF
        -DASSIMP_BUILD_TESTS=OFF
        -DASSIMP_BUILD_IFC_IMPORTER=OFF
        -DASSIMP_BUILD_ZLIB=OFF
        -DASSIMP_INSTALL_PDB=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIR}
        -DZLIB_LIBRARY_DBG=${ZLIB_LIBRARY_DBG}
        -DZLIB_LIBRARY_REL=${ZLIB_LIBRARY_REL}
        -DZLIB_FOUND=YES
)

ExternalProject_Get_Property(ext-assimp BINARY_DIR)
set(ASSIMP_BUILDDIR "${BINARY_DIR}")
message(STATUS "Assimp Build directory is ${ASSIMP_BUILDDIR}")

add_library(assimp-lib INTERFACE)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp-bin/lib/cmake/assimp-3.3/assimp-config.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp-bin/lib/cmake/assimp-3.3/assimp-config.cmake)

    target_include_directories(assimp-lib INTERFACE ${ASSIMP_INCLUDE_DIRS})
    target_link_libraries(assimp-lib INTERFACE ${ASSIMP_LIBRARIES})
    link_directories(${ASSIMP_LIBRARY_DIRS})

    target_link_libraries(assimp-lib INTERFACE ZLIB)
else ()
    message(STATUS "***** Assimp is not built yet. Build the ext-assimp target, and then reconfigure")
endif ()


add_library(GSL INTERFACE)
target_include_directories(GSL INTERFACE 3rdparty/GSL/include)

add_subdirectory(3rdparty/gameplay)
add_subdirectory(src)
