find_package( Threads REQUIRED )

include( wrap_enum )

wrap_enum(
        NAMESPACE loader::file
        NAME AnimationId
        TYPE uint16_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/animationids.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/loader/file/animationid.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/loader/file/animationid.cpp
)
wrap_enum(
        NAMESPACE loader::file
        NAME LaraStateId
        TYPE uint16_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/states.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/loader/file/larastateid.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/loader/file/larastateid.cpp
)
wrap_enum(
        NAMESPACE engine
        NAME TR1ItemId
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/items_tr1.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/engine/items_tr1.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/engine/items_tr1.cpp
)
wrap_enum(
        NAMESPACE engine
        NAME TR1SoundEffect
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/soundeffects_tr1.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/engine/soundeffects_tr1.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/engine/soundeffects_tr1.cpp
)
wrap_enum(
        NAMESPACE engine
        NAME TR1TrackId
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tracks_tr1.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/engine/tracks_tr1.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/engine/tracks_tr1.cpp
)
wrap_enum(
        NAMESPACE hid
        NAME GlfwKey
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/glfw_keys.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_keys.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_keys.cpp
        WITH_INCLUDES "<GLFW/glfw3.h>"
)
wrap_enum(
        NAMESPACE hid
        NAME GlfwGamepadButton
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/glfw_gamepad_buttons.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_gamepad_buttons.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_gamepad_buttons.cpp
        WITH_INCLUDES "<GLFW/glfw3.h>"
)
wrap_enum(
        NAMESPACE hid
        NAME GlfwAxis
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/glfw_axes.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_axes.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_axes.cpp
        WITH_INCLUDES "<GLFW/glfw3.h>"
)
wrap_enum(
        NAMESPACE hid
        NAME GlfwAxisDir
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/glfw_axis_dirs.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_axis_dirs.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/hid/glfw_axis_dirs.cpp
)
wrap_enum(
        NAMESPACE hid
        NAME Action
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/actions.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/hid/actions.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/hid/actions.cpp
)
wrap_enum(
        NAMESPACE engine::ghosting
        NAME GhostFinishState
        TYPE int32_t
        SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/ghostfinishstate.txt
        DEST ${CMAKE_CURRENT_SOURCE_DIR}/engine/ghosting/ghostfinishstate.h
        DEST_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/engine/ghosting/ghostfinishstate.cpp
)

add_library(
        serialization
        STATIC
        serialization/array.h
        serialization/bitset.h
        serialization/deque.h
        serialization/glm.h
        serialization/map.h
        serialization/not_null.h
        serialization/objectreference.h
        serialization/optional.h
        serialization/ptr.h
        serialization/quantity.h
        serialization/serialization.h
        serialization/serialization.cpp
        serialization/serialization_fwd.h
        serialization/skeletalmodeltype_ptr.h
        serialization/unordered_map.h
        serialization/unordered_set.h
        serialization/vector.h
)
target_include_directories( serialization PUBLIC . )
target_link_libraries(
        serialization
        PUBLIC
        Boost::headers
        Boost::log
        ryml
        gsl-lite::gsl-lite
)

add_library(
        gameflow
        STATIC
        gameflow/meta.h
        gameflow/meta.cpp
)
target_include_directories( gameflow PUBLIC . )
target_link_libraries(
        gameflow
        PUBLIC
        serialization
)

set( CROFTENGINE_SRCS
        croftengine.cpp
        gslfailhandler.cpp

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
        engine/audiosettings.h
        engine/audiosettings.cpp
        engine/cameracontroller.h
        engine/cameracontroller.cpp
        engine/controllerbuttons.h
        engine/controllerbuttons.cpp
        engine/collisioninfo.h
        engine/collisioninfo.cpp
        engine/displaysettings.h
        engine/displaysettings.cpp
        engine/engine.h
        engine/engine.cpp
        engine/engineconfig.h
        engine/engineconfig.cpp
        engine/ghostmanager.h
        engine/ghostmanager.cpp
        engine/heightinfo.h
        engine/heightinfo.cpp
        engine/inventory.h
        engine/inventory.cpp
        engine/lighting.h
        engine/lighting.cpp
        engine/location.h
        engine/location.cpp
        engine/objectmanager.h
        engine/objectmanager.cpp
        engine/particle.h
        engine/particle.cpp
        engine/player.h
        engine/player.cpp
        engine/presenter.h
        engine/presenter.cpp
        engine/py_module.cpp
        engine/raycast.h
        engine/raycast.cpp
        engine/skeletalmodelnode.h
        engine/skeletalmodelnode.cpp
        engine/items_tr1.cpp
        engine/soundeffects_tr1.cpp
        engine/tracks_tr1.cpp

        engine/world/box.h
        engine/world/box.cpp
        engine/world/camerasink.h
        engine/world/camerasink.cpp
        engine/world/rendermeshdata.h
        engine/world/rendermeshdata.cpp
        engine/world/room.h
        engine/world/room.cpp
        engine/world/sector.h
        engine/world/sector.cpp
        engine/world/world.h
        engine/world/world.cpp
        engine/world/texturing.h
        engine/world/texturing.cpp

        engine/script/reflection.h
        engine/script/reflection.cpp
        engine/script/scriptengine.h
        engine/script/scriptengine.cpp

        hid/inputstate.h
        hid/inputhandler.h
        hid/inputhandler.cpp
        hid/names.h
        hid/names.cpp
        hid/actions.cpp
        hid/glfw_axes.cpp
        hid/glfw_axis_dirs.cpp
        hid/glfw_gamepad_buttons.cpp
        hid/glfw_keys.cpp

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
        loader/file/animationid.cpp
        loader/file/larastateid.cpp

        loader/file/io/sdlreader.h
        loader/file/io/util.h

        loader/trx/trx.h
        loader/trx/trx.cpp

        audio/loadefx.h
        audio/loadefx.cpp
        audio/bufferhandle.h
        audio/bufferhandle.cpp
        audio/buffervoice.h
        audio/buffervoice.cpp
        audio/device.h
        audio/device.cpp
        audio/emitter.h
        audio/emitter.cpp
        audio/filterhandle.h
        audio/listener.h
        audio/listener.cpp
        audio/soundengine.h
        audio/soundengine.cpp
        audio/sourcehandle.h
        audio/sourcehandle.cpp
        audio/streamvoice.h
        audio/streamvoice.cpp
        audio/streamsource.h
        audio/wadstreamsource.cpp
        audio/utils.h
        audio/utils.cpp
        audio/voice.h
        audio/voice.cpp
        audio/voicegroup.h
        audio/voicegroup.cpp

        util/helpers.h
        util/helpers.cpp
        util/md5.h
        util/md5.cpp

        engine/objects/aiagent.cpp
        engine/objects/aiagent.h
        engine/objects/aiminfo.cpp
        engine/objects/aiminfo.h
        engine/objects/animating.cpp
        engine/objects/animating.h
        engine/objects/atlanteanlava.cpp
        engine/objects/atlanteanlava.h
        engine/objects/barricade.cpp
        engine/objects/barricade.h
        engine/objects/bat.cpp
        engine/objects/bat.h
        engine/objects/bear.cpp
        engine/objects/bear.h
        engine/objects/block.cpp
        engine/objects/block.h
        engine/objects/bridgeflat.h
        engine/objects/centaurstatue.cpp
        engine/objects/centaurstatue.h
        engine/objects/collapsiblefloor.cpp
        engine/objects/collapsiblefloor.h
        engine/objects/cowboy.cpp
        engine/objects/cowboy.h
        engine/objects/crocodile.cpp
        engine/objects/crocodile.h
        engine/objects/cutsceneactors.cpp
        engine/objects/cutsceneactors.h
        engine/objects/dart.cpp
        engine/objects/dart.h
        engine/objects/dartgun.cpp
        engine/objects/dartgun.h
        engine/objects/door.cpp
        engine/objects/door.h
        engine/objects/doppelganger.cpp
        engine/objects/doppelganger.h
        engine/objects/earthquake.cpp
        engine/objects/fallingceiling.cpp
        engine/objects/fallingceiling.h
        engine/objects/flameemitter.cpp
        engine/objects/flameemitter.h
        engine/objects/gorilla.cpp
        engine/objects/gorilla.h
        engine/objects/keyhole.cpp
        engine/objects/keyhole.h
        engine/objects/kold.cpp
        engine/objects/kold.h
        engine/objects/laraobject.cpp
        engine/objects/laraobject.h
        engine/objects/larson.cpp
        engine/objects/larson.h
        engine/objects/lavaparticleemitter.cpp
        engine/objects/lavaparticleemitter.h
        engine/objects/lightningemitter.cpp
        engine/objects/lightningemitter.h
        engine/objects/lion.cpp
        engine/objects/lion.h
        engine/objects/midasgoldtouch.cpp
        engine/objects/midasgoldtouch.h
        engine/objects/modelobject.cpp
        engine/objects/modelobject.h
        engine/objects/motorboat.cpp
        engine/objects/motorboat.h
        engine/objects/mummy.cpp
        engine/objects/mummy.h
        engine/objects/mutant.cpp
        engine/objects/mutant.h
        engine/objects/mutantegg.cpp
        engine/objects/mutantegg.h
        engine/objects/natla.cpp
        engine/objects/natla.h
        engine/objects/object.cpp
        engine/objects/object.h
        engine/objects/objectfactory.cpp
        engine/objects/objectfactory.h
        engine/objects/objectstate.cpp
        engine/objects/objectstate.h
        engine/objects/pickupobject.cpp
        engine/objects/pickupobject.h
        engine/objects/pierre.cpp
        engine/objects/pierre.h
        engine/objects/puzzlehole.cpp
        engine/objects/puzzlehole.h
        engine/objects/raptor.cpp
        engine/objects/raptor.h
        engine/objects/rat.cpp
        engine/objects/rat.h
        engine/objects/rollingball.cpp
        engine/objects/rollingball.h
        engine/objects/scionpiece.cpp
        engine/objects/scionpiece.h
        engine/objects/shatter.cpp
        engine/objects/shatter.h
        engine/objects/skateboardkid.cpp
        engine/objects/skateboardkid.h
        engine/objects/slammingdoors.cpp
        engine/objects/slammingdoors.h
        engine/objects/slopedbridge.cpp
        engine/objects/slopedbridge.h
        engine/objects/spriteobject.cpp
        engine/objects/spriteobject.h
        engine/objects/stubobject.h
        engine/objects/suspendedshack.cpp
        engine/objects/suspendedshack.h
        engine/objects/swingingblade.cpp
        engine/objects/swingingblade.h
        engine/objects/switch.cpp
        engine/objects/switch.h
        engine/objects/swordofdamocles.cpp
        engine/objects/swordofdamocles.h
        engine/objects/tallblock.cpp
        engine/objects/tallblock.h
        engine/objects/teethspikes.cpp
        engine/objects/teethspikes.h
        engine/objects/thorhammer.cpp
        engine/objects/thorhammer.h
        engine/objects/trapdoordown.cpp
        engine/objects/trapdoordown.h
        engine/objects/trapdoordown.h
        engine/objects/trapdoorup.cpp
        engine/objects/trapdoorup.h
        engine/objects/trex.cpp
        engine/objects/trex.h
        engine/objects/underwaterswitch.cpp
        engine/objects/underwaterswitch.h
        engine/objects/waterfallmist.cpp
        engine/objects/waterfallmist.h
        engine/objects/weapon.cpp
        engine/objects/weapon.h
        engine/objects/wolf.cpp
        engine/objects/wolf.h

        engine/ai/ai.h
        engine/ai/ai.cpp
        engine/ai/pathfinder.h
        engine/ai/pathfinder.cpp

        engine/floordata/floordata.h
        engine/floordata/floordata.cpp
        engine/floordata/types.h
        engine/floordata/secrets.h
        engine/floordata/secrets.cpp

        engine/ghosting/ghost.h
        engine/ghosting/ghost.cpp
        engine/ghosting/ghostmodel.h
        engine/ghosting/ghostmodel.cpp
        engine/ghosting/ghostfinishstate.h
        engine/ghosting/ghostfinishstate.cpp

        render/portaltracer.h
        render/portaltracer.cpp
        render/renderpipeline.h
        render/renderpipeline.cpp
        render/rendersettings.h
        render/rendersettings.cpp
        render/textureanimator.h
        render/textureanimator.cpp

        render/pass/effectpass.h
        render/pass/framebuffer.h
        render/pass/framebuffer.cpp
        render/pass/geometrypass.h
        render/pass/geometrypass.cpp
        render/pass/hbaopass.h
        render/pass/hbaopass.cpp
        render/pass/portalpass.h
        render/pass/portalpass.cpp
        render/pass/uipass.h
        render/pass/uipass.cpp
        render/pass/worldcompositionpass.h
        render/pass/worldcompositionpass.cpp

        render/scene/bufferparameter.h
        render/scene/bufferparameter.cpp
        render/scene/camera.h
        render/scene/csm.h
        render/scene/csm.cpp
        render/scene/material.h
        render/scene/material.cpp
        render/scene/materialgroup.h
        render/scene/materialmanager.h
        render/scene/materialmanager.cpp
        render/scene/materialparameter.h
        render/scene/mesh.h
        render/scene/mesh.cpp
        render/scene/names.h
        render/scene/node.h
        render/scene/node.cpp
        render/scene/renderable.h
        render/scene/rendercontext.h
        render/scene/renderer.h
        render/scene/renderer.cpp
        render/scene/rendermode.h
        render/scene/screenoverlay.h
        render/scene/screenoverlay.cpp
        render/scene/shadercache.h
        render/scene/shadercache.cpp
        render/scene/shaderprogram.h
        render/scene/shaderprogram.cpp
        render/scene/sprite.h
        render/scene/sprite.cpp
        render/scene/uniformparameter.h
        render/scene/uniformparameter.cpp
        render/scene/visitor.h
        render/scene/visitor.cpp

        ui/text.h
        ui/text.cpp
        ui/levelstats.h
        ui/levelstats.cpp
        ui/ui.h
        ui/ui.cpp

        ui/widgets/checkbox.cpp
        ui/widgets/gridbox.cpp
        ui/widgets/groupbox.cpp
        ui/widgets/label.cpp
        ui/widgets/listbox.cpp
        ui/widgets/messagebox.cpp
        ui/widgets/progressbar.cpp
        ui/widgets/selectionbox.cpp
        ui/widgets/sprite.cpp
        ui/widgets/tabbox.cpp

        video/audiostreamdecoder.h
        video/audiostreamdecoder.cpp
        video/ffmpegstreamsource.h
        video/ffmpegstreamsource.cpp
        video/avdecoder.h
        video/avdecoder.cpp
        video/avframeptr.h
        video/avframeptr.cpp
        video/converter.h
        video/converter.cpp
        video/filtergraph.h
        video/filtergraph.cpp
        video/videoplayer.h
        video/videoplayer.cpp
        video/stream.h
        video/stream.cpp

        core/angle.h
        core/angle.cpp
        core/boundingbox.h
        core/containeroffset.h
        core/i18n.cpp
        core/id.h
        core/magic.h
        core/py_module.cpp
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

        menu/menudisplay.cpp
        menu/menuobject.cpp
        menu/audiosettingsmenustate.cpp
        menu/applyitemtransformmenustate.cpp
        menu/closepassportmenustate.cpp
        menu/compassmenustate.h
        menu/compassmenustate.cpp
        menu/controlsmenustate.cpp
        menu/controlswidget.cpp
        menu/deflateringmenustate.cpp
        menu/deselectingmenustate.cpp
        menu/donemenustate.cpp
        menu/finishitemanimationmenustate.cpp
        menu/idleringmenustate.cpp
        menu/inflateringmenustate.cpp
        menu/listdisplaymenustate.cpp
        menu/passportmenustate.cpp
        menu/rendersettingsmenustate.cpp
        menu/requestloadmenustate.cpp
        menu/resetitemtransformmenustate.cpp
        menu/rotateleftrightmenustate.cpp
        menu/savegamelistmenustate.cpp
        menu/selectedmenustate.cpp
        menu/setitemtypemenustate.cpp
        menu/switchringmenustate.cpp
        menu/util.cpp
        )

if( MSVC )
    list( APPEND CROFTENGINE_SRCS croftengine.rc )
endif()

file(
        GLOB_RECURSE ALL_SRCS
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
        *.cpp
        *.h
        *.hpp
)

set( CE_ADDITIONAL_LANGUAGES )

file( STRINGS languages.txt _languages )
foreach( _language IN LISTS _languages )
    if( NOT _language MATCHES ".+ .+" )
        continue()
    endif()

    string( REGEX REPLACE " .+\$" "" _lang_id "${_language}" )

    if( _lang_id STREQUAL "en_GB" )
        continue()
    endif()

    list( APPEND CE_ADDITIONAL_LANGUAGES ${_lang_id} )
endforeach()


configure_gettext(
        TARGET_NAME i18n
        DOMAIN croftengine
        SOURCES ${ALL_SRCS}
        POTFILE_DESTINATION "${CMAKE_SOURCE_DIR}/share"
        LANGUAGES ${CE_ADDITIONAL_LANGUAGES}
        XGETTEXT_ARGS
        "--keyword=_"
        "--keyword=N_"
        "--keyword=P_:1,2"
        "--package-name=${PROJECT_NAME}"
        "--package-version=${PROJECT_VERSION}"
        "--copyright-holder=Steffen Ohrendorf"
        "--msgid-bugs-address=https://github.com/stohrendorf/CroftEngine/issues"
)

add_executable( croftengine ${CROFTENGINE_SRCS} )

set_property(
        SOURCE croftengine.cpp
        PROPERTY COMPILE_DEFINITIONS CE_VERSION="${CMAKE_PROJECT_VERSION}"
)

group_files( ${CROFTENGINE_SRCS} )

target_include_directories( croftengine PRIVATE . ${Intl_INCLUDE_DIRS} )

add_subdirectory( shared )
add_subdirectory( soglb )
add_subdirectory( qs )
add_subdirectory( core )
add_subdirectory( launcher )
add_subdirectory( dosbox-cdrom )

target_link_libraries(
        croftengine
        PRIVATE
        Boost::system
        Boost::locale
        Boost::log
        Boost::log_setup
        Boost::disable_autolinking
        Boost::headers
        Boost::stacktrace
        OpenAL::OpenAL
        ryml
        PNG::PNG
        type_safe
        OpenGL
        glm::glm
        FFmpeg
        gsl-lite::gsl-lite
        soglb
        pybind11::pybind11
        pybind11::embed
        Python3::Python
        Threads::Threads
        ${Intl_LIBRARIES}
        LibArchive::LibArchive
        shared
        launcher
        serialization
)

install(
        TARGETS croftengine
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(( LINUX OR UNIX ) AND CMAKE_COMPILER_IS_GNUCC )
    target_link_libraries(
            croftengine
            PRIVATE
            stdc++fs
    )
endif()

get_target_property( _bin_dir croftengine BINARY_DIR )

add_custom_target( croftengine-runtime-deps )
add_dependencies( croftengine croftengine-runtime-deps )

file(
        GLOB_RECURSE _shared_files
        RELATIVE ${CMAKE_SOURCE_DIR}/share
        ${CMAKE_SOURCE_DIR}/share/*
)
list( FILTER _shared_files EXCLUDE REGEX "\\.full\\." )
list( FILTER _shared_files EXCLUDE REGEX "\\.po" )
list( FILTER _shared_files EXCLUDE REGEX "__pycache__" )

foreach( _file ${_shared_files} )
    get_filename_component( _dirname ${_file} DIRECTORY )
    add_custom_command(
            COMMENT "Copy ${_file}"
            OUTPUT ${_bin_dir}/share/${_file}
            MAIN_DEPENDENCY ${CMAKE_SOURCE_DIR}/share/${_file}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${_bin_dir}/share/croftengine/${_dirname}
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/share/${_file} ${_bin_dir}/share/croftengine/${_dirname}/
    )
    set_source_files_properties(
            ${_bin_dir}/share/${_file}
            PROPERTIES GENERATED TRUE
    )
    set_property(
            TARGET croftengine-runtime-deps
            APPEND PROPERTY SOURCES ${_bin_dir}/share/${_file}
    )
    install(
            FILES ${CMAKE_SOURCE_DIR}/share/${_file}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/croftengine/${_dirname}
    )
endforeach()

if( WIN32 AND NOT DEFINED ENV{WSLENV} )
    copy_python3_deps( croftengine )

    file(
            GLOB _dlls
            RELATIVE ${_bin_dir}
            ${_bin_dir}/*.dll
    )
    foreach( _dll ${_dlls} )
        install(
                FILES ${_bin_dir}/${_dll}
                DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endforeach()

    file(
            GLOB _pdbs
            RELATIVE ${_bin_dir}
            ${_bin_dir}/*.pdb
    )
    foreach( _pdb ${_pdbs} )
        install(
                FILES ${_bin_dir}/${_pdb}
                DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endforeach()

    file(
            GLOB_RECURSE _qt5_plugins
            RELATIVE ${_bin_dir}
            ${_bin_dir}/plugins/**/*.*
    )
    foreach( _file ${_qt5_plugins} )
        get_filename_component( _dirname ${_file} DIRECTORY )
        install(
                FILES ${_bin_dir}/${_file}
                DESTINATION ${CMAKE_INSTALL_BINDIR}/${_dirname}
        )
    endforeach()
endif()
