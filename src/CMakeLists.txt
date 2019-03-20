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
    file( APPEND "${dest}" "inline const char* toString(${name} val)\n" )
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
wrap_enum( ControllerAxis int ${CMAKE_CURRENT_SOURCE_DIR}/controlleraxes.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/controlleraxis_enum.h )
wrap_enum( ControllerButton int ${CMAKE_CURRENT_SOURCE_DIR}/controllerbuttons.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/controllerbutton_enum.h )
wrap_enum( TR1ItemId int ${CMAKE_CURRENT_SOURCE_DIR}/items_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/items_tr1_enum.h )
wrap_enum( TR1SoundId int ${CMAKE_CURRENT_SOURCE_DIR}/sounds_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/sounds_tr1_enum.h )
wrap_enum( TR1TrackId int ${CMAKE_CURRENT_SOURCE_DIR}/tracks_tr1.txt ${CMAKE_CURRENT_SOURCE_DIR}/engine/tracks_tr1_enum.h )

set( EDISONENGINE_SRCS
     edisonengine.cpp

     engine/engine.cpp
     engine/lara/abstractstatehandler.cpp
     engine/cameracontroller.cpp
     engine/collisioninfo.cpp
     engine/heightinfo.cpp
     engine/laranode.cpp
     engine/particle.cpp
     engine/skeletalmodelnode.cpp
     engine/audioengine.cpp
     engine/inventory.cpp

     loader/file/level/level.cpp
     loader/file/level/tr1level.cpp
     loader/file/level/tr2level.cpp
     loader/file/level/tr3level.cpp
     loader/file/level/tr4level.cpp
     loader/file/level/tr5level.cpp

     loader/file/datatypes.cpp
     loader/file/mesh.cpp
     loader/file/texture.cpp
     loader/trx/trx.cpp

     audio/alext.cpp
     audio/device.cpp
     audio/utils.cpp
     audio/stream.cpp
     audio/soundengine.cpp

     util/md5.cpp
     util/cimgwrapper.cpp

     engine/lara/abstractstatehandler.cpp

     engine/items/animating.cpp
     engine/items/bear.cpp
     engine/items/barricade.cpp
     engine/items/bat.cpp
     engine/items/block.cpp
     engine/items/boulder.cpp
     engine/items/crocodile.cpp
     engine/items/collapsiblefloor.cpp
     engine/items/cutsceneactors.cpp
     engine/items/dart.cpp
     engine/items/door.cpp
     engine/items/flameemitter.cpp
     engine/items/gorilla.cpp
     engine/items/itemnode.cpp
     engine/items/keyhole.cpp
     engine/items/larson.cpp
     engine/items/lightningball.cpp
     engine/items/lion.cpp
     engine/items/mummy.cpp
     engine/items/pickupitem.cpp
     engine/items/pierre.cpp
     engine/items/puzzlehole.cpp
     engine/items/raptor.cpp
     engine/items/scionpiece.cpp
     engine/items/swingingblade.cpp
     engine/items/switch.cpp
     engine/items/swordofdamocles.cpp
     engine/items/tallblock.cpp
     engine/items/teethspikes.cpp
     engine/items/thorhammer.cpp
     engine/items/trapdoorup.cpp
     engine/items/trex.cpp
     engine/items/underwaterswitch.cpp
     engine/items/waterfallmist.cpp
     engine/items/wolf.cpp
     engine/items/aiagent.cpp

     engine/ai/ai.cpp

     render/label.cpp
     render/textureanimator.cpp

     render/gl/util.h
     render/gl/shader.h
     render/gl/program.h
     render/gl/bindableresource.h
     render/gl/framebuffer.h
     render/gl/indexbuffer.h
     render/gl/pixel.h
     render/gl/renderbuffer.h
     render/gl/rendertarget.h
     render/gl/texture.h
     render/gl/typetraits.h
     render/gl/vertexarray.h
     render/gl/vertexbuffer.h
     render/gl/debuggroup.h
     render/gl/font.cpp
     render/gl/font.h
     render/gl/image.h
     render/gl/structuredvertexbuffer.h
     render/gl/vertexattribute.h

     render/scene/Camera.cpp
     render/scene/Game.cpp
     render/scene/Material.cpp
     render/scene/MaterialParameter.cpp
     render/scene/Mesh.cpp
     render/scene/MeshPart.cpp
     render/scene/Model.cpp
     render/scene/Node.cpp
     render/scene/RenderState.cpp
     render/scene/ScreenOverlay.cpp
     render/scene/ShaderProgram.cpp
     render/scene/Sprite.cpp
     render/scene/Visitor.cpp
     )

if( MSVC )
    list( APPEND EDISONENGINE_SRCS edisonengine.rc )
endif()

add_definitions(
        -DSOL_USE_BOOST
        -DSOL_SAFE_USERTYPE
        -DSOL_SAFE_FUNCTION
)

add_executable( edisonengine
                ${EDISONENGINE_SRCS}
                )

group_files( ${EDISONENGINE_SRCS} )

target_include_directories( edisonengine PRIVATE . )

target_link_libraries(
        edisonengine
        PRIVATE
        Boost::boost
        ZLIB::ZLIB
        OpenAL::OpenAL
        sndfile-shared
        CImg::CImg
        Lua
        yaml-cpp
        GLEW::GLEW
        glfw
        PNG::PNG
        sol2
        type_safe
        GLEW::GLEW
        glfw
        Boost::boost
        OpenGL
        glm
        Freetype::Freetype
)

if( LINUX OR UNIX )
    target_link_libraries(
            edisonengine
            PRIVATE
            pthread
    )
endif()

add_subdirectory(qs)