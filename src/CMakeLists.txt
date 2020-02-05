function( wrap_enum name type source dest )
    if( EXISTS "${dest}" AND "${dest}" IS_NEWER_THAN "${source}" )
        return()
    endif()
    message( STATUS "Generating enum include ${dest} from ${source}" )
    file( STRINGS "${source}" _vals )
    file( WRITE "${dest}" "" )
    # Write enum
    file( APPEND "${dest}" "enum class ${name} : ${type}\n" )
    file( APPEND "${dest}" "{\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()

        # remove "= 123" part
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        if( _val MATCHES ".+=.+" )
            # extract value of "= 123" part
            string( REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}" )
            # emit: Value = IntValue,
            file( APPEND "${dest}" "    ${_ename} = ${_eval},\n" )
        else()
            # emit: Value,
            file( APPEND "${dest}" "    ${_ename},\n" )
        endif()
    endforeach()
    file( APPEND "${dest}" "};\n\n" )

    # toString function
    file( APPEND "${dest}" "inline gsl::czstring toString(${name} val)\n" )
    file( APPEND "${dest}" "{\n" )
    file( APPEND "${dest}" "    switch(val)\n" )
    file( APPEND "${dest}" "    {\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()
        # remove "= 123" part
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        # emit: case Enum::Value: return "Value";
        file( APPEND "${dest}" "        case ${name}::${_ename}: return \"${_ename}\";\n" )
    endforeach()
    file( APPEND "${dest}" "        default: return nullptr;\n" )
    file( APPEND "${dest}" "    }\n" )
    file( APPEND "${dest}" "}\n\n" )

    # EnumUtil forward declaration
    file( APPEND "${dest}" "template<typename T> struct EnumUtil;\n\n" )
    # EnumUtil specialization
    file( APPEND "${dest}" "template<> struct EnumUtil<${name}>\n" )
    file( APPEND "${dest}" "{\n" )

    # fromString()
    file( APPEND "${dest}" "    static ${name} fromString(const std::string& value)\n" )
    file( APPEND "${dest}" "    {\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        if( _eval STREQUAL "" )
            continue()
        endif()
        # emit: if(value == "Value") return Enum::Value;
        file( APPEND "${dest}" "        if(value == \"${_ename}\") return ${name}::${_ename};\n" )
    endforeach()
    file( APPEND "${dest}" "        throw std::domain_error(\"Value '\" + value + \"' invalid for enum '${name}'\");\n" )
    file( APPEND "${dest}" "    }\n" )

    # all()
    file( APPEND "${dest}" "    static std::map<${name}, std::string> all()\n" )
    file( APPEND "${dest}" "    {\n" )
    file( APPEND "${dest}" "        return {\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        if( _eval STREQUAL "" )
            continue()
        endif()
        # emit: { enum, string },
        file( APPEND "${dest}" "            {${name}::${_ename}, \"${_ename}\"},\n" )
    endforeach()
    file( APPEND "${dest}" "        };\n" )
    file( APPEND "${dest}" "    }\n" )

    file( APPEND "${dest}" "};\n" )
endfunction()

wrap_enum( AnimationId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/animationids.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/file/animationid_enum.h )
wrap_enum( LaraStateId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/states.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/file/larastateid_enum.h )
wrap_enum( TR1ItemId int ${CMAKE_CURRENT_SOURCE_DIR}/items_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/items_tr1_enum.h )
wrap_enum( TR1SoundId int ${CMAKE_CURRENT_SOURCE_DIR}/sounds_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/sounds_tr1_enum.h )
wrap_enum( TR1TrackId int ${CMAKE_CURRENT_SOURCE_DIR}/tracks_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/tracks_tr1_enum.h )

set( EDISONENGINE_SRCS
        edisonengine.cpp

        engine/lara/abstractstatehandler.h
        engine/lara/abstractstatehandler.cpp
        engine/lara/statehandler_0.h
        engine/lara/statehandler_1.h
        engine/lara/statehandler_2.h
        engine/lara/statehandler_3.h
        engine/lara/statehandler_4.h
        engine/lara/statehandler_5.h
        engine/lara/statehandler_6.h
        engine/lara/statehandler_7.h
        engine/lara/statehandler_8.h
        engine/lara/statehandler_9.h
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
        engine/lara/statehandler_50.h
        engine/lara/statehandler_51.h
        engine/lara/statehandler_52.h
        engine/lara/statehandler_53.h
        engine/lara/statehandler_54.h
        engine/lara/statehandler_55.h
        engine/lara/statehandler_onwater.h
        engine/lara/statehandler_pushable.h
        engine/lara/statehandler_standing.h
        engine/lara/statehandler_turnslow.h
        engine/lara/statehandler_underwater.h

        engine/audioengine.h
        engine/audioengine.cpp
        engine/cameracontroller.h
        engine/cameracontroller.cpp
        engine/collisioninfo.h
        engine/collisioninfo.cpp
        engine/engine.h
        engine/engine.cpp
        engine/heightinfo.h
        engine/heightinfo.cpp
        engine/inventory.h
        engine/inventory.cpp
        engine/lighting.h
        engine/particle.h
        engine/particle.cpp
        engine/skeletalmodelnode.h
        engine/skeletalmodelnode.cpp

        engine/script/reflection.h

        hid/inputstate.h
        hid/inputhandler.h
        hid/inputhandler.cpp

        loader/file/level/game.h
        loader/file/level/level.h
        loader/file/level/level.cpp
        loader/file/level/tr1level.h
        loader/file/level/tr1level.cpp
        loader/file/level/tr2level.h
        loader/file/level/tr2level.cpp
        loader/file/level/tr3level.h
        loader/file/level/tr3level.cpp
        loader/file/level/tr4level.h
        loader/file/level/tr4level.cpp
        loader/file/level/tr5level.h
        loader/file/level/tr5level.cpp
        loader/file/animation.h
        loader/file/animation.cpp
        loader/file/audio.h
        loader/file/audio.cpp
        loader/file/color.h
        loader/file/color.cpp
        loader/file/datatypes.h
        loader/file/datatypes.cpp
        loader/file/item.h
        loader/file/item.cpp
        loader/file/mesh.h
        loader/file/mesh.cpp
        loader/file/meshes.h
        loader/file/meshes.cpp
        loader/file/primitives.h
        loader/file/primitives.cpp
        loader/file/texture.h
        loader/file/texture.cpp
        loader/file/texturecache.h
        loader/file/util.h

        loader/file/io/sdlreader.h
        loader/file/io/util.h

        loader/trx/trx.h
        loader/trx/trx.cpp

        audio/alext.h
        audio/alext.cpp
        audio/bufferhandle.h
        audio/device.h
        audio/device.cpp
        audio/filterhandle.h
        audio/soundengine.h
        audio/soundengine.cpp
        audio/stream.h
        audio/stream.cpp
        audio/streamsource.h
        audio/tracktype.h
        audio/utils.h
        audio/utils.cpp

        util/helpers.h
        util/md5.h
        util/md5.cpp

        engine/objects/objectfactory.h
        engine/objects/objectfactory.cpp
        engine/objects/animating.h
        engine/objects/animating.cpp
        engine/objects/atlanteanlava.h
        engine/objects/atlanteanlava.cpp
        engine/objects/bear.h
        engine/objects/bear.cpp
        engine/objects/barricade.h
        engine/objects/barricade.cpp
        engine/objects/bat.h
        engine/objects/bat.cpp
        engine/objects/block.h
        engine/objects/block.cpp
        engine/objects/boulder.h
        engine/objects/boulder.cpp
        engine/objects/crocodile.h
        engine/objects/crocodile.cpp
        engine/objects/collapsiblefloor.h
        engine/objects/collapsiblefloor.cpp
        engine/objects/cutsceneactors.h
        engine/objects/cutsceneactors.cpp
        engine/objects/dart.h
        engine/objects/dart.cpp
        engine/objects/dartgun.h
        engine/objects/dartgun.cpp
        engine/objects/door.h
        engine/objects/door.cpp
        engine/objects/doppelganger.h
        engine/objects/doppelganger.cpp
        engine/objects/flameemitter.h
        engine/objects/flameemitter.cpp
        engine/objects/gorilla.h
        engine/objects/gorilla.cpp
        engine/objects/object.h
        engine/objects/object.cpp
        engine/objects/objectstate.h
        engine/objects/objectstate.cpp
        engine/objects/keyhole.h
        engine/objects/keyhole.cpp
        engine/objects/laraobject.h
        engine/objects/laraobject.cpp
        engine/objects/larson.h
        engine/objects/larson.cpp
        engine/objects/lavaparticleemitter.h
        engine/objects/lavaparticleemitter.cpp
        engine/objects/lightningball.h
        engine/objects/lightningball.cpp
        engine/objects/lion.h
        engine/objects/lion.cpp
        engine/objects/modelobject.h
        engine/objects/modelobject.cpp
        engine/objects/mummy.h
        engine/objects/mummy.cpp
        engine/objects/mutant.h
        engine/objects/mutant.cpp
        engine/objects/mutantegg.h
        engine/objects/mutantegg.cpp
        engine/objects/pickupobject.h
        engine/objects/pickupobject.cpp
        engine/objects/pierre.h
        engine/objects/pierre.cpp
        engine/objects/puzzlehole.h
        engine/objects/puzzlehole.cpp
        engine/objects/raptor.h
        engine/objects/raptor.cpp
        engine/objects/rat.h
        engine/objects/rat.cpp
        engine/objects/scionpiece.h
        engine/objects/scionpiece.cpp
        engine/objects/slammingdoors.h
        engine/objects/slammingdoors.cpp
        engine/objects/spriteobject.h
        engine/objects/spriteobject.cpp
        engine/objects/swingingblade.h
        engine/objects/swingingblade.cpp
        engine/objects/switch.h
        engine/objects/switch.cpp
        engine/objects/swordofdamocles.h
        engine/objects/swordofdamocles.cpp
        engine/objects/tallblock.h
        engine/objects/tallblock.cpp
        engine/objects/teethspikes.h
        engine/objects/teethspikes.cpp
        engine/objects/thorhammer.h
        engine/objects/thorhammer.cpp
        engine/objects/trapdoorup.h
        engine/objects/trapdoorup.cpp
        engine/objects/trex.h
        engine/objects/trex.cpp
        engine/objects/underwaterswitch.h
        engine/objects/underwaterswitch.cpp
        engine/objects/waterfallmist.h
        engine/objects/waterfallmist.cpp
        engine/objects/wolf.h
        engine/objects/wolf.cpp
        engine/objects/aiagent.h
        engine/objects/aiagent.cpp
        engine/objects/bridgeflat.h
        engine/objects/slopedbridge.h
        engine/objects/stubobject.h
        engine/objects/trapdoordown.h

        engine/ai/ai.h
        engine/ai/ai.cpp
        engine/ai/pathfinder.h
        engine/ai/pathfinder.cpp

        engine/floordata/floordata.h
        engine/floordata/floordata.cpp
        engine/floordata/types.h

        render/portaltracer.h
        render/renderpipeline.h
        render/renderpipeline.cpp
        render/textureanimator.h
        render/textureanimator.cpp

        render/scene/bufferparameter.h
        render/scene/bufferparameter.cpp
        render/scene/camera.h
        render/scene/csm.h
        render/scene/csm.cpp
        render/scene/material.h
        render/scene/material.cpp
        render/scene/materialmanager.h
        render/scene/materialmanager.cpp
        render/scene/materialparameter.h
        render/scene/mesh.h
        render/scene/mesh.cpp
        render/scene/model.h
        render/scene/model.cpp
        render/scene/multipassmaterial.h
        render/scene/names.h
        render/scene/node.h
        render/scene/node.cpp
        render/scene/renderable.h
        render/scene/rendercontext.h
        render/scene/renderer.h
        render/scene/renderer.cpp
        render/scene/rendermode.h
        render/scene/rendervisitor.h
        render/scene/scene.h
        render/scene/screenoverlay.h
        render/scene/screenoverlay.cpp
        render/scene/shadermanager.h
        render/scene/shadermanager.cpp
        render/scene/shaderprogram.h
        render/scene/shaderprogram.cpp
        render/scene/sprite.h
        render/scene/sprite.cpp
        render/scene/uniformparameter.h
        render/scene/uniformparameter.cpp
        render/scene/visitor.h
        render/scene/visitor.cpp

        ui/label.h
        ui/label.cpp

        video/player.h
        video/player.cpp

        core/angle.h
        core/angle.cpp
        core/boundingbox.h
        core/containeroffset.h
        core/id.h
        core/magic.h
        core/tpl_helper.h
        core/units.h
        core/vec.h
        core/vec.cpp

        qs/mult_div.h
        qs/qs.h
        qs/quantity.h
        qs/stream.h
        qs/string_util.h
        qs/tuple_util.h

        serialization/animation_ptr.h
        serialization/animframe_ptr.h
        serialization/array.h
        serialization/bitset.h
        serialization/box_ptr.h
        serialization/deque.h
        serialization/glm.h
        serialization/map.h
        serialization/not_null.h
        serialization/objectreference.h
        serialization/optional.h
        serialization/ptr.h
        serialization/quantity.h
        serialization/room_ptr.h
        serialization/serialization.h
        serialization/serialization.cpp
        serialization/serialization_fwd.h
        serialization/skeletalmodeltype_ptr.h
        serialization/sprite_ptr.h
        serialization/unordered_map.h
        serialization/unordered_set.h
        serialization/vector.h
        )

if( MSVC )
    list( APPEND EDISONENGINE_SRCS edisonengine.rc )
endif()

set( EDISONENGINE_UI_SRCS
        ui/debug/debugview.cpp
        ui/debug/larainfowidget.cpp
        ui/debug/laraaiminfo.cpp
        )

qt5_wrap_cpp( EDISONENGINE_UI_SRCS
        ui/debug/debugview.h
        ui/debug/larainfowidget.h
        ui/debug/laraaiminfo.h
        )

add_definitions(
        -DSOL_USE_BOOST
        -DSOL_SAFE_USERTYPE
        -DSOL_SAFE_FUNCTION
)

add_executable( edisonengine
        ${EDISONENGINE_SRCS}
        ${EDISONENGINE_UI_SRCS}
        )

group_files( ${EDISONENGINE_SRCS} ${EDISONENGINE_UI_SRCS} )

target_include_directories( edisonengine PRIVATE . )

add_subdirectory( soglb )
add_subdirectory( qs )

target_link_libraries(
        edisonengine
        PRIVATE
        Boost::boost
        ZLIB::ZLIB
        OpenAL::OpenAL
        sndfile-shared
        Lua
        yaml-cpp
        PNG::PNG
        sol2::sol2
        type_safe
        OpenGL
        glm
        FFmpeg
        gsl-lite
        soglb
        Qt5::Widgets
)

if( LINUX OR UNIX )
    target_link_libraries(
            edisonengine
            PRIVATE
            pthread
    )

    if( CMAKE_COMPILER_IS_GNUCC )
        target_link_libraries(
                edisonengine
                PRIVATE
                stdc++fs
        )
    endif()
endif()
