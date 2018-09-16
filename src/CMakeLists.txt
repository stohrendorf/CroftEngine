function(wrap_enum name type source dest)
    if (EXISTS "${dest}" AND "${dest}" IS_NEWER_THAN "${source}")
        return()
    endif ()
    message(STATUS "Generating enum include ${dest} from ${source}")
    file(STRINGS "${source}" _vals)
    file(WRITE "${dest}" "")
    # Write enum
    file(APPEND "${dest}" "enum class ${name} : ${type}\n")
    file(APPEND "${dest}" "{\n")
    foreach (_val IN LISTS _vals)
        # remove trailing whitespace/comments
        string(REGEX REPLACE "//.*\$" "" _val "${_val}")
        string(REPLACE " " "" _val "${_val}")
        string(REPLACE "\t" "" _val "${_val}")
        if (_val STREQUAL "")
            continue()
        endif ()

        # remove "= 123" part
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if (_ename STREQUAL "")
            continue()
        endif ()
        if (_val MATCHES ".+=.+")
            # extract value of "= 123" part
            string(REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}")
            # emit: Value = IntValue,
            file(APPEND "${dest}" "    ${_ename} = ${_eval},\n")
        else ()
            # emit: Value,
            file(APPEND "${dest}" "    ${_ename},\n")
        endif ()
    endforeach ()
    file(APPEND "${dest}" "};\n\n")

    # toString function
    file(APPEND "${dest}" "inline const char* toString(${name} val)\n")
    file(APPEND "${dest}" "{\n")
    file(APPEND "${dest}" "    switch(val)\n")
    file(APPEND "${dest}" "    {\n")
    foreach (_val IN LISTS _vals)
        # remove trailing whitespace/comments
        string(REGEX REPLACE "//.*\$" "" _val "${_val}")
        string(REPLACE " " "" _val "${_val}")
        string(REPLACE "\t" "" _val "${_val}")
        if (_val STREQUAL "")
            continue()
        endif ()
        # remove "= 123" part
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if (_ename STREQUAL "")
            continue()
        endif ()
        if (_val MATCHES ".+=.+")
            # extract value of "= 123" part
            string(REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}")
            # emit: case Enum::Value: return "Value(IntValue)";
            file(APPEND "${dest}" "        case ${name}::${_ename}: return \"${_ename}(${_eval})\";\n")
        else ()
            # emit: case Enum::Value: return "Value";
            file(APPEND "${dest}" "        case ${name}::${_ename}: return \"${_ename}\";\n")
        endif ()
    endforeach ()
    file(APPEND "${dest}" "        default: return nullptr;\n")
    file(APPEND "${dest}" "    }\n")
    file(APPEND "${dest}" "}\n\n")

    # EnumUtil forward declaration
    file(APPEND "${dest}" "template<typename T> struct EnumUtil;\n\n")
    # EnumUtil specialization
    file(APPEND "${dest}" "template<> struct EnumUtil<${name}>\n")
    file(APPEND "${dest}" "{\n")
    file(APPEND "${dest}" "    static ${name} fromString(const std::string& value)\n")
    file(APPEND "${dest}" "    {\n")
    foreach (_val IN LISTS _vals)
        # remove trailing whitespace/comments
        string(REGEX REPLACE "//.*\$" "" _val "${_val}")
        string(REPLACE " " "" _val "${_val}")
        string(REPLACE "\t" "" _val "${_val}")
        if (_val STREQUAL "")
            continue()
        endif ()
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if (_ename STREQUAL "")
            continue()
        endif ()
        if (_eval STREQUAL "")
            continue()
        endif ()
        # emit: if(value == "Value") return Enum::Value;
        file(APPEND "${dest}" "        if(value == \"${_ename}\") return ${name}::${_ename};\n")
    endforeach ()
    file(APPEND "${dest}" "        throw std::domain_error(\"Value '\" + value + \"' invalid for enum '${name}'\");\n")
    file(APPEND "${dest}" "    }\n")
    file(APPEND "${dest}" "};\n")
endfunction()

wrap_enum(AnimationId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/animationids.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/animationid_enum.h)
wrap_enum(LaraStateId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/states.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/larastateid_enum.h)
wrap_enum(ControllerAxis int ${CMAKE_CURRENT_SOURCE_DIR}/controlleraxes.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/controlleraxis_enum.h)
wrap_enum(ControllerButton int ${CMAKE_CURRENT_SOURCE_DIR}/controllerbuttons.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/controllerbutton_enum.h)
wrap_enum(TR1ItemId int ${CMAKE_CURRENT_SOURCE_DIR}/items_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/items_tr1_enum.h)

set( EDISONENGINE_SRCS
     edisonengine.cpp

     core/angle.h
     core/boundingbox.h
     core/vec.h
     core/magic.h

     engine/lara/abstractstatehandler.h
     engine/lara/abstractstatehandler.cpp
     engine/cameracontroller.h
     engine/cameracontroller.cpp
     engine/collisioninfo.cpp
     engine/collisioninfo.h
     engine/controlleraxis.h
     engine/controllerbutton.h
     engine/controllerlayout.h
     engine/heightinfo.cpp
     engine/heightinfo.h
     engine/inputhandler.h
     engine/inputstate.h
     engine/laranode.h
     engine/laranode.cpp
     engine/particle.h
     engine/particle.cpp
     engine/skeletalmodelnode.h
     engine/skeletalmodelnode.cpp

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
     loader/animationid.h
     loader/audio.h
     loader/color.h
     loader/datatypes.cpp
     loader/datatypes.h
     loader/item.h
     loader/larastateid.h
     loader/meshes.h
     loader/mesh.cpp
     loader/mesh.h
     loader/converter.h
     loader/converter.cpp
     loader/primitives.h
     loader/texture.h
     loader/texture.cpp
     loader/util.h
     loader/io/sdlreader.h
     loader/trx/trx.h
     loader/trx/trx.cpp

     render/portaltracer.h
     render/textureanimator.h

     audio/alext.h
     audio/alext.cpp
     audio/device.h
     audio/device.cpp
     audio/utils.h
     audio/utils.cpp
     audio/bufferhandle.h
     audio/filterhandle.h
     audio/sourcehandle.h
     audio/streamsource.h
     audio/stream.h
     audio/stream.cpp
     audio/sndfile/helpers.h

     util/helpers.h
     util/md5.h
     util/md5.cpp
     util/cimgwrapper.h
     util/cimgwrapper.cpp

     engine/lara/abstractstatehandler.cpp
     engine/lara/abstractstatehandler.h
     engine/lara/statehandler_0.h
     engine/lara/statehandler_1.h
     engine/lara/statehandler_10.h
     engine/lara/statehandler_11.h
     engine/lara/statehandler_12.h
     engine/lara/statehandler_13.h
     engine/lara/statehandler_14.h
     engine/lara/statehandler_15.h
     engine/lara/statehandler_16.h
     engine/lara/statehandler_17.h
     engine/lara/statehandler_18.h
     engine/lara/statehandler_19.h
     engine/lara/statehandler_2.h
     engine/lara/statehandler_20.h
     engine/lara/statehandler_21.h
     engine/lara/statehandler_22.h
     engine/lara/statehandler_23.h
     engine/lara/statehandler_24.h
     engine/lara/statehandler_25.h
     engine/lara/statehandler_26.h
     engine/lara/statehandler_27.h
     engine/lara/statehandler_28.h
     engine/lara/statehandler_29.h
     engine/lara/statehandler_3.h
     engine/lara/statehandler_30.h
     engine/lara/statehandler_31.h
     engine/lara/statehandler_32.h
     engine/lara/statehandler_33.h
     engine/lara/statehandler_34.h
     engine/lara/statehandler_35.h
     engine/lara/statehandler_36.h
     engine/lara/statehandler_37.h
     engine/lara/statehandler_38.h
     engine/lara/statehandler_39.h
     engine/lara/statehandler_4.h
     engine/lara/statehandler_40.h
     engine/lara/statehandler_41.h
     engine/lara/statehandler_42.h
     engine/lara/statehandler_43.h
     engine/lara/statehandler_44.h
     engine/lara/statehandler_45.h
     engine/lara/statehandler_46.h
     engine/lara/statehandler_47.h
     engine/lara/statehandler_48.h
     engine/lara/statehandler_49.h
     engine/lara/statehandler_5.h
     engine/lara/statehandler_50.h
     engine/lara/statehandler_51.h
     engine/lara/statehandler_52.h
     engine/lara/statehandler_53.h
     engine/lara/statehandler_54.h
     engine/lara/statehandler_55.h
     engine/lara/statehandler_6.h
     engine/lara/statehandler_7.h
     engine/lara/statehandler_8.h
     engine/lara/statehandler_9.h
     engine/lara/statehandler_onwater.h
     engine/lara/statehandler_pushable.h
     engine/lara/statehandler_standing.h
     engine/lara/statehandler_turnslow.h
     engine/lara/statehandler_underwater.h

     engine/items/animating.cpp
     engine/items/animating.h
     engine/items/bear.cpp
     engine/items/bear.h
     engine/items/bat.cpp
     engine/items/bat.h
     engine/items/block.cpp
     engine/items/block.h
     engine/items/boulder.h
     engine/items/boulder.cpp
     engine/items/bridgeflat.h
     engine/items/collapsiblefloor.cpp
     engine/items/collapsiblefloor.h
     engine/items/cutsceneactors.cpp
     engine/items/cutsceneactors.h
     engine/items/dart.h
     engine/items/dart.cpp
     engine/items/dartgun.h
     engine/items/door.h
     engine/items/itemnode.cpp
     engine/items/itemnode.h
     engine/items/keyhole.cpp
     engine/items/keyhole.h
     engine/items/pickupitem.h
     engine/items/pickupitem.cpp
     engine/items/puzzlehole.h
     engine/items/puzzlehole.cpp
     engine/items/raptor.h
     engine/items/raptor.cpp
     engine/items/scionpiece.h
     engine/items/scionpiece.cpp
     engine/items/slopedbridge.h
     engine/items/stubitem.h
     engine/items/swingingblade.cpp
     engine/items/swingingblade.h
     engine/items/switch.cpp
     engine/items/switch.h
     engine/items/swordofdamocles.cpp
     engine/items/swordofdamocles.h
     engine/items/tallblock.cpp
     engine/items/tallblock.h
     engine/items/teethspikes.cpp
     engine/items/teethspikes.h
     engine/items/trapdoordown.h
     engine/items/trapdoorup.cpp
     engine/items/trapdoorup.h
     engine/items/underwaterswitch.h
     engine/items/underwaterswitch.cpp
     engine/items/wolf.h
     engine/items/wolf.cpp
     engine/items/aiagent.h
     engine/items/aiagent.cpp

     engine/floordata/floordata.h
     engine/ai/ai.h
     engine/ai/ai.cpp

     render/label.cpp
)

if(MSVC)
    list(APPEND EDISONENGINE_SRCS edisonengine.rc)
endif()

add_definitions(
        -DSOL_USE_BOOST
        -DSOL_SAFE_USERTYPE
        -DSOL_SAFE_FUNCTION
)

add_executable(edisonengine
        ${EDISONENGINE_SRCS}
        )

GROUP_FILES(${EDISONENGINE_SRCS})

target_include_directories(edisonengine PRIVATE .)

if (NOT MSVC)
    target_compile_options(edisonengine PRIVATE -Wall -Wextra)
endif ()

target_link_libraries(
        edisonengine
        PRIVATE
        Boost::boost
        gameplay
        ZLIB::ZLIB
        OpenAL::OpenAL
        sndfile-shared
        CImg::CImg
        Lua
        yaml-cpp
        GLEW::GLEW
        glfw
        Assimp
        PNG::PNG
        sol2
)

if (LINUX OR UNIX)
    target_link_libraries(
            edisonengine
            pthread
    )
endif ()
