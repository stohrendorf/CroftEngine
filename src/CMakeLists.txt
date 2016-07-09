function(gen_enum name type source dest)
    if(EXISTS "${dest}" AND "${dest}" IS_NEWER_THAN "${source}")
        return()
    endif()
    message(STATUS "Generating enum include ${dest} from ${source}")
    file(STRINGS "${source}" _vals)
    file(WRITE "${dest}" "")
    # Write enum
    file(APPEND "${dest}" "enum class ${name} : ${type}\n")
    file(APPEND "${dest}" "{\n")
    foreach(_val IN LISTS _vals)
        string(REGEX REPLACE "\\s*//.*" "" _val "${_val}")
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if("${_ename}" STREQUAL "")
            continue()
        endif()
        string(REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}")
        if("${_eval}" STREQUAL "")
            continue()
        endif()
        file(APPEND "${dest}" "    ${_ename} = ${_eval},\n")
    endforeach()
    file(APPEND "${dest}" "};\n\n")
    # Write toString
    file(APPEND "${dest}" "inline const char* toString(${name} val)\n")
    file(APPEND "${dest}" "{\n")
    file(APPEND "${dest}" "    switch(val)\n")
    file(APPEND "${dest}" "    {\n")
    foreach(_val IN LISTS _vals)
        string(REGEX REPLACE "\\s*//.*" "" _val "${_val}")
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if("${_ename}" STREQUAL "")
            continue()
        endif()
        string(REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}")
        if("${_eval}" STREQUAL "")
            continue()
        endif()
        file(APPEND "${dest}" "        case ${name}::${_ename}: return \"${_ename}(${_eval})\";\n")
    endforeach()
    file(APPEND "${dest}" "        default: return nullptr;\n")
    file(APPEND "${dest}" "    }\n")
    file(APPEND "${dest}" "}\n")
endfunction()

gen_enum(AnimationId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/animationids.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/animationids_enum.h)
gen_enum(LaraStateId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/states.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/larastateids_enum.h)

set(OT_IRR_SRCS
    ot_irr.cpp

    core/angle.h
    core/coordinates.h
    core/interpolatedvalue.h
    core/magic.h

    engine/abstractstatehandler.cpp
    engine/abstractstatehandler.h
    engine/animationcontroller.cpp
    engine/animationcontroller.h
    engine/cameracontroller.cpp
    engine/cameracontroller.h
    engine/heightinfo.cpp
    engine/heightinfo.h
    engine/inputhandler.h
    engine/inputstate.h
    engine/itemcontroller.cpp
    engine/itemcontroller.h
    engine/laracontroller.cpp
    engine/laracontroller.h
    engine/larastate.cpp
    engine/larastate.h

    level/game.h
    level/level.cpp
    level/level.h
    level/tr1level.cpp
    level/tr1level.h
    level/tr2level.cpp
    level/tr2level.h
    level/tr3level.cpp
    level/tr3level.h
    level/tr4level.cpp
    level/tr4level.h
    level/tr5level.cpp
    level/tr5level.h

    loader/animation.h
    loader/audio.h
    loader/color.h
    loader/datatypes.cpp
    loader/datatypes.h
    loader/floordata.h
    loader/item.h
    loader/larastateid.h
    loader/meshes.cpp
    loader/meshes.h
    loader/primitives.h
    loader/texture.h
    loader/util.h

    render/lightselector.h
    render/portaltracer.h
    render/textureanimator.h

    util/helpers.h
    util/vmath.h

    ../3rdparty/xeffects/Source/CScreenQuad.h
    ../3rdparty/xeffects/Source/CShaderPre.h
    ../3rdparty/xeffects/Source/CShaderPre.cpp
    ../3rdparty/xeffects/Source/EffectCB.h
    ../3rdparty/xeffects/Source/EffectHandler.h
    ../3rdparty/xeffects/Source/EffectHandler.cpp
    ../3rdparty/xeffects/Source/EffectShaders.h
    ../3rdparty/xeffects/Source/XEffects.h
)

add_executable(ot_irr
    ${OT_IRR_SRCS}
)

GROUP_FILES(${OT_IRR_SRCS})

if(NOT MSVC)
    target_compile_options(ot_irr PRIVATE -std=c++14 -Wall -Wextra -msse3)
endif()

target_link_libraries(
    ot_irr
    Boost
    Irrlicht
    SDL2
    ZLIB
)

target_include_directories(ot_irr PRIVATE . ../3rdparty/GSL/include ../3rdparty/xeffects/Source)
