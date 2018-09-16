cmake_minimum_required(VERSION 3.11.2)
project(EdisonEngine)

set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    # C4503: Name too long
    # C4996: "Call to 'std::copy_n' with parameters that may be unsafe" etc...
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4503 /wd4996 /MP")
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

function(GROUP_FILES)
    foreach (fn ${ARGV})
        get_filename_component(parentDir "${fn}" PATH)

        # change /'s to \\'s, and reduce redundant // or \\.
        string(REGEX REPLACE "[\\/]+" "\\\\" grp "${parentDir}")

        source_group("${grp}" FILES "${fn}")
    endforeach ()
endfunction()

find_package(OpenAL CONFIG REQUIRED)

find_package(OpenGL REQUIRED)
add_library(OpenGL INTERFACE)
target_include_directories(OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})
target_link_libraries(OpenGL INTERFACE ${OPENGL_LIBRARIES})

find_path(LUA_INCLUDE_DIR lua.hpp)
find_library(LUA_LIBRARIES lua)
add_library(Lua INTERFACE)
target_link_libraries(Lua INTERFACE ${LUA_LIBRARIES})
target_include_directories(Lua INTERFACE ${LUA_INCLUDE_DIR})

find_package(glm CONFIG REQUIRED)
target_compile_definitions(glm INTERFACE -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS)

find_package(Boost COMPONENTS system log filesystem thread REQUIRED)
target_compile_definitions(Boost::boost INTERFACE -DBOOST_LOG_DYN_LINK=0)
target_compile_definitions(Boost::boost INTERFACE -DBOOST_ALL_NO_LIB)
target_include_directories(Boost::boost INTERFACE ${Boost_INCLUDE_DIR})
target_link_libraries(Boost::boost INTERFACE ${Boost_LIBRARIES})

find_package(ZLIB REQUIRED)

find_package(PNG REQUIRED)

find_package(LibSndFile REQUIRED)

find_package(Freetype REQUIRED)

find_package(GLEW REQUIRED)

find_package(glfw3 REQUIRED)

find_package(CImg REQUIRED)
target_compile_definitions(CImg::CImg INTERFACE -Dcimg_display=0 -Dcimg_use_png)

find_package(sol2 CONFIG REQUIRED)

find_package(Git REQUIRED)
execute_process(
        # COMMAND ${GIT_EXECUTABLE} describe --dirty --tags --long
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        OUTPUT_VARIABLE GIT_SHA
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_definitions(-DGIT_SHA="${GIT_SHA}")

find_package(yaml-cpp REQUIRED)

add_subdirectory(3rdparty/gameplay)
add_subdirectory(src)
