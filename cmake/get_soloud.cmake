if( TARGET soloud )
    return()
endif()

include( ./dl_unpack )

dl_unpack( http://sol.gfxile.net/soloud/soloud_20200207_lite.zip soloud_20200207_lite.zip soloud20200207 )

find_package( Threads REQUIRED )

file(
        GLOB_RECURSE _SOLOUD_CORE_SRCS
        "${EXTERNAL_SRC_ROOT}/soloud20200207/src/core/*.c"
        "${EXTERNAL_SRC_ROOT}/soloud20200207/src/core/*.cpp"
        "${EXTERNAL_SRC_ROOT}/soloud20200207/src/audiosource/wav/*.c"
        "${EXTERNAL_SRC_ROOT}/soloud20200207/src/audiosource/wav/*.cpp"
)
add_library(
        soloud STATIC
        ${_SOLOUD_CORE_SRCS}
        "${EXTERNAL_SRC_ROOT}/soloud20200207/src/filter/soloud_biquadresonantfilter.cpp"
)
target_include_directories( soloud PUBLIC "${EXTERNAL_SRC_ROOT}/soloud20200207/include" )
target_link_directories( soloud PUBLIC Threads::Threads ${CMAKE_DL_LIBS} )

function( add_soloud_backend name definition )
    file(
            GLOB_RECURSE _SOLOUD_BACKEND_SRCS
            "${EXTERNAL_SRC_ROOT}/soloud20200207/src/backend/${name}/*.c"
            "${EXTERNAL_SRC_ROOT}/soloud20200207/src/backend/${name}/*.cpp"
    )
    add_library(
            soloud_backend_${name} STATIC
            ${_SOLOUD_BACKEND_SRCS}
    )
    target_include_directories( soloud_backend_${name} PUBLIC "${EXTERNAL_SRC_ROOT}/soloud20200207/include" )
    target_compile_definitions( soloud_backend_${name} PUBLIC -DWITH_${definition} )
    target_link_libraries( soloud PUBLIC soloud_backend_${name} )
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
add_soloud_backend( openal OPENAL )
target_link_libraries( soloud_backend_openal PRIVATE OpenAL::OpenAL )

if( WIN32 )
    add_soloud_backend( wasapi WASAPI )
    add_soloud_backend( winmm WINMM )
    add_soloud_backend( xaudio2 XAUDIO2 )
elseif( LINUX OR UNIX )
    add_soloud_backend( alsa ALSA )
    add_soloud_backend( oss OSS )

    find_package( ALSA REQUIRED )
    target_link_libraries( soloud_backend_alsa PRIVATE ALSA::ALSA )
elseif( APPLE )
    add_soloud_backend( portaudio PORTAUDIO )
else()
    message( FATAL_ERROR "Cannot determine audio backend" )
endif()
