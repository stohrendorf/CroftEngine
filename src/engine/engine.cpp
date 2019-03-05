#include "engine.h"

#include "floordata/floordata.h"
#include "loader/file/level/level.h"

#include "engine/items/animating.h"
#include "engine/items/barricade.h"
#include "engine/items/bat.h"
#include "engine/items/bear.h"
#include "engine/items/block.h"
#include "engine/items/boulder.h"
#include "engine/items/bridgeflat.h"
#include "engine/items/collapsiblefloor.h"
#include "engine/items/crocodile.h"
#include "engine/items/cutsceneactors.h"
#include "engine/items/dart.h"
#include "engine/items/dartgun.h"
#include "engine/items/door.h"
#include "engine/items/flameemitter.h"
#include "engine/items/gorilla.h"
#include "engine/items/keyhole.h"
#include "engine/items/larson.h"
#include "engine/items/lightningball.h"
#include "engine/items/lion.h"
#include "engine/items/mummy.h"
#include "engine/items/pickupitem.h"
#include "engine/items/pierre.h"
#include "engine/items/puzzlehole.h"
#include "engine/items/raptor.h"
#include "engine/items/scionpiece.h"
#include "engine/items/slopedbridge.h"
#include "engine/items/stubitem.h"
#include "engine/items/swingingblade.h"
#include "engine/items/switch.h"
#include "engine/items/swordofdamocles.h"
#include "engine/items/tallblock.h"
#include "engine/items/teethspikes.h"
#include "engine/items/thorhammer.h"
#include "engine/items/trapdoordown.h"
#include "engine/items/trapdoorup.h"
#include "engine/items/trex.h"
#include "engine/items/underwaterswitch.h"
#include "engine/items/waterfallmist.h"
#include "engine/items/wolf.h"

#include "engine/laranode.h"

#include "audio/tracktype.h"
#include "render/textureanimator.h"

#include "gl/font.h"
#include "render/fullscreenfx.h"
#include "render/label.h"
#include "loader/trx/trx.h"

#include <boost/range/adaptor/map.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <glm/gtx/norm.hpp>

namespace engine
{
namespace
{
sol::state createScriptEngine()
{
    sol::state engine;
    engine.open_libraries( sol::lib::base, sol::lib::math, sol::lib::package );
    engine["package"]["path"] = (boost::filesystem::path( "scripts" ) / "?.lua").string();
    engine["package"]["cpath"] = "";

    engine.set_usertype( core::Angle::userType() );
    engine.set_usertype( core::TRRotation::userType() );
    engine.set_usertype( core::TRVec::userType() );
    engine.set_usertype( engine::ai::CreatureInfo::userType() );
    engine.set_usertype( engine::items::ItemState::userType() );

    engine.new_enum( "ActivationState",
                     "INACTIVE", engine::items::TriggerState::Inactive,
                     "ACTIVE", engine::items::TriggerState::Active,
                     "DEACTIVATED", engine::items::TriggerState::Deactivated,
                     "INVISIBLE", engine::items::TriggerState::Invisible
    );

    engine.new_enum( "Mood",
                     "BORED", engine::ai::Mood::Bored,
                     "ATTACK", engine::ai::Mood::Attack,
                     "ESCAPE", engine::ai::Mood::Escape,
                     "STALK", engine::ai::Mood::Stalk
    );

    engine.new_enum( "TrackType",
                     "AMBIENT", audio::TrackType::Ambient,
                     "INTERCEPTION", audio::TrackType::Interception,
                     "AMBIENT_EFFECT", audio::TrackType::AmbientEffect,
                     "LARA_TALK", audio::TrackType::LaraTalk
    );

    {
        sol::table tbl = engine.create_table( "TR1SoundId" );
        for( const auto& entry : engine::EnumUtil<engine::TR1SoundId>::all() )
            tbl[entry.second] = static_cast<std::underlying_type_t<engine::TR1SoundId>>(entry.first);
    }

    {
        sol::table tbl = engine.create_table( "TR1TrackId" );
        for( const auto& entry : engine::EnumUtil<engine::TR1TrackId>::all() )
            tbl[entry.second] = static_cast<std::underlying_type_t<engine::TR1TrackId>>(entry.first);
    }

    return engine;
}
}

std::tuple<int8_t, int8_t>
Engine::getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector, const core::TRVec& position) const
{
    while( sector->roomBelow != nullptr )
    {
        sector = sector->roomBelow->getSectorByAbsolutePosition( position );
    }

    static const auto zero = std::make_tuple( 0, 0 );

    if( position.Y + core::QuarterSectorSize * 2 < sector->floorHeight )
        return zero;
    if( sector->floorData == nullptr )
        return zero;
    if( floordata::FloorDataChunk{*sector->floorData}.type
        != floordata::FloorDataChunkType::FloorSlant )
        return zero;

    const auto fd = sector->floorData[1];
    return std::make_tuple( gsl::narrow_cast<int8_t>( fd.get() & 0xff ),
                            gsl::narrow_cast<int8_t>( fd.get() >> 8 ) );
}

void Engine::swapAllRooms()
{
    BOOST_LOG_TRIVIAL( info ) << "Swapping rooms";
    for( auto& room : m_level->m_rooms )
    {
        if( room.alternateRoom.get() < 0 )
            continue;

        BOOST_ASSERT( static_cast<size_t>(room.alternateRoom.get()) < m_level->m_rooms.size() );
        swapWithAlternate( room, m_level->m_rooms.at( room.alternateRoom.get() ) );
    }

    roomsAreSwapped = !roomsAreSwapped;
}

bool Engine::isValid(const loader::file::AnimFrame* frame) const
{
    return reinterpret_cast<const short*>(frame) >= m_level->m_poseFrames.data()
           && reinterpret_cast<const short*>(frame) < m_level->m_poseFrames.data() + m_level->m_poseFrames.size();
}

const std::unique_ptr<loader::file::SpriteSequence>& Engine::findSpriteSequenceForType(TR1ItemId type) const
{
    return m_level->findSpriteSequenceForType( type );
}

const loader::file::StaticMesh* Engine::findStaticMeshById(core::StaticMeshId meshId) const
{
    return m_level->findStaticMeshById( meshId );
}

const std::vector<loader::file::Room>& Engine::getRooms() const
{
    return m_level->m_rooms;
}

const std::vector<loader::file::Box>& Engine::getBoxes() const
{
    return m_level->m_boxes;
}

std::map<loader::file::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>
Engine::createMaterials(const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader)
{
    const auto texMask = gameToEngine( m_level->m_gameVersion ) == loader::file::level::Engine::TR4
                         ? loader::file::TextureIndexMaskTr4
                         : loader::file::TextureIndexMask;
    std::map<loader::file::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>> materials;
    for( loader::file::TextureLayoutProxy& proxy : m_level->m_textureProxies )
    {
        const auto& key = proxy.textureKey;
        if( materials.find( key ) != materials.end() )
            continue;

        materials
                .emplace( key, proxy.createMaterial( m_level->m_textures[key.tileAndFlag & texMask].texture, shader ) );
    }
    return materials;
}

std::shared_ptr<LaraNode> Engine::createItems()
{
    m_lightningShader = gameplay::ShaderProgram::createFromFile( "shaders/lightning.vert", "shaders/lightning.frag" );

    std::shared_ptr<LaraNode> lara = nullptr;
    int id = -1;
    for( loader::file::Item& item : m_level->m_items )
    {
        ++id;

        const auto* room = &m_level->m_rooms.at( item.room.get() );

        if( const auto& model = findAnimatedModelForType( item.type ) )
        {
            std::shared_ptr<items::ItemNode> modelNode;

            if( item.type == TR1ItemId::Lara )
            {
                lara = std::make_shared<LaraNode>( this, room, item, *model );
                modelNode = lara;
            }
            else if( auto objectInfo = m_scriptEngine["getObjectInfo"].call( -1 ) )
            {
                BOOST_LOG_TRIVIAL( info ) << "Instantiating scripted type " << toString( item.type ) << "/id "
                                          << id;

                modelNode = std::make_shared<items::ScriptedItem>( this,
                                                                   room,
                                                                   item,
                                                                   *model,
                                                                   objectInfo );
                for( gsl::index boneIndex = 0; boneIndex < model->models.size(); ++boneIndex )
                {
                    auto node = std::make_shared<gameplay::Node>(
                            modelNode->getNode()->getId() + "/bone:" + std::to_string( boneIndex ) );
                    node->setDrawable( model->models[boneIndex].get() );
                    addChild( modelNode->getNode(), node );
                }

                BOOST_ASSERT(
                        modelNode->getNode()->getChildren().size() == gsl::narrow<size_t>( model->meshes.size() ) );
            }
            else if( item.type == TR1ItemId::Wolf )
            {
                modelNode = std::make_shared<items::Wolf>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Bear )
            {
                modelNode = std::make_shared<items::Bear>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Bat )
            {
                modelNode = std::make_shared<items::Bat>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::FallingBlock )
            {
                modelNode = std::make_shared<items::CollapsibleFloor>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::SwingingBlade )
            {
                modelNode = std::make_shared<items::SwingingBlade>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::RollingBall )
            {
                modelNode = std::make_shared<items::RollingBall>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Dart )
            {
                modelNode = std::make_shared<items::Dart>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::DartEmitter )
            {
                modelNode = std::make_shared<items::DartGun>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::LiftingDoor )
            {
                modelNode = std::make_shared<items::TrapDoorUp>( this, room, item, *model );
            }
            else if( item.type >= TR1ItemId::PushableBlock1 && item.type <= TR1ItemId::PushableBlock4 )
            {
                modelNode = std::make_shared<items::Block>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::MovingBlock )
            {
                modelNode = std::make_shared<items::TallBlock>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::WallSwitch )
            {
                modelNode = std::make_shared<items::Switch>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::UnderwaterSwitch )
            {
                modelNode = std::make_shared<items::UnderwaterSwitch>( this, room, item, *model );
            }
            else if( item.type >= TR1ItemId::Door1 && item.type <= TR1ItemId::Door8 )
            {
                modelNode = std::make_shared<items::Door>( this, room, item, *model );
            }
            else if( item.type >= TR1ItemId::Trapdoor1 && item.type <= TR1ItemId::Trapdoor2 )
            {
                modelNode = std::make_shared<items::TrapDoorDown>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::BridgeFlat )
            {
                modelNode = std::make_shared<items::BridgeFlat>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::BridgeTilt1 )
            {
                modelNode = std::make_shared<items::BridgeSlope1>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::BridgeTilt2 )
            {
                modelNode = std::make_shared<items::BridgeSlope2>( this, room, item, *model );
            }
            else if( item.type >= TR1ItemId::Keyhole1 && item.type <= TR1ItemId::Keyhole4 )
            {
                modelNode = std::make_shared<items::KeyHole>( this, room, item, *model );
            }
            else if( item.type >= TR1ItemId::PuzzleHole1 && item.type <= TR1ItemId::PuzzleHole4 )
            {
                modelNode = std::make_shared<items::PuzzleHole>( this, room, item, *model );
            }
            else if( item.type >= TR1ItemId::Animating1 && item.type <= TR1ItemId::Animating3 )
            {
                modelNode = std::make_shared<items::Animating>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::TeethSpikes )
            {
                modelNode = std::make_shared<items::TeethSpikes>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Raptor )
            {
                modelNode = std::make_shared<items::Raptor>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::SwordOfDamocles || item.type == TR1ItemId::FallingCeiling )
            {
                modelNode = std::make_shared<items::SwordOfDamocles>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::CutsceneActor1 )
            {
                modelNode = std::make_shared<items::CutsceneActor1>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::CutsceneActor2 )
            {
                modelNode = std::make_shared<items::CutsceneActor2>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::CutsceneActor3 )
            {
                modelNode = std::make_shared<items::CutsceneActor3>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::CutsceneActor4 )
            {
                modelNode = std::make_shared<items::CutsceneActor4>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::WaterfallMist )
            {
                modelNode = std::make_shared<items::WaterfallMist>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::TRex )
            {
                modelNode = std::make_shared<items::TRex>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Mummy )
            {
                modelNode = std::make_shared<items::Mummy>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Larson )
            {
                modelNode = std::make_shared<items::Larson>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::CrocodileOnLand
                     || item.type == TR1ItemId::CrocodileInWater )
            {
                modelNode = std::make_shared<items::Crocodile>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::LionMale
                     || item.type == TR1ItemId::LionFemale
                     || item.type == TR1ItemId::Panther )
            {
                modelNode = std::make_shared<items::Lion>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Barricade )
            {
                modelNode = std::make_shared<items::Barricade>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Gorilla )
            {
                modelNode = std::make_shared<items::Gorilla>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::Pierre )
            {
                modelNode = std::make_shared<items::Pierre>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::ThorHammerBlock )
            {
                modelNode = std::make_shared<items::ThorHammerBlock>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::ThorHammerHandle )
            {
                modelNode = std::make_shared<items::ThorHammerHandle>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::FlameEmitter )
            {
                modelNode = std::make_shared<items::FlameEmitter>( this, room, item, *model );
            }
            else if( item.type == TR1ItemId::ThorLightningBall )
            {
                modelNode = std::make_shared<items::LightningBall>( this, room, item,
                                                                    *model,
                                                                    m_lightningShader );
            }
            else
            {
                BOOST_LOG_TRIVIAL( warning ) << "Unimplemented item " << toString( item.type );

                modelNode = std::make_shared<items::StubItem>( this, room, item, *model );
                if( item.type == TR1ItemId::MidasGoldTouch
                    || item.type == TR1ItemId::CameraTarget
                    || item.type == TR1ItemId::LavaParticleEmitter
                    || item.type == TR1ItemId::FlameEmitter
                    || item.type == TR1ItemId::Earthquake )
                {
                    modelNode->getNode()->setDrawable( nullptr );
                    modelNode->getNode()->removeAllChildren();
                }
            }

            m_itemNodes.emplace( std::make_pair( id, modelNode ) );
            addChild( room->node, modelNode->getNode() );

            modelNode->applyTransform();
            modelNode->updateLighting();

            continue;
        }

        if( const auto& spriteSequence = findSpriteSequenceForType( item.type ) )
        {
            BOOST_ASSERT( !findAnimatedModelForType( item.type ) );
            BOOST_ASSERT( !spriteSequence->sprites.empty() );

            const loader::file::Sprite& sprite = spriteSequence->sprites[0];
            std::shared_ptr<items::ItemNode> node;

            if( item.type == TR1ItemId::ScionPiece1 )
            {
                node = std::make_shared<items::ScionPieceItem>( this,
                                                                std::string( "sprite(type:" )
                                                                + toString( item.type ) + ")",
                                                                room,
                                                                item,
                                                                sprite,
                                                                m_spriteMaterial );
            }
            else if( item.type == TR1ItemId::Item141
                     || item.type == TR1ItemId::Item142
                     || item.type == TR1ItemId::Key1Sprite
                     || item.type == TR1ItemId::Key2Sprite
                     || item.type == TR1ItemId::Key3Sprite
                     || item.type == TR1ItemId::Key4Sprite
                     || item.type == TR1ItemId::Puzzle1Sprite
                     || item.type == TR1ItemId::Puzzle2Sprite
                     || item.type == TR1ItemId::Puzzle3Sprite
                     || item.type == TR1ItemId::Puzzle4Sprite
                     || item.type == TR1ItemId::PistolsSprite
                     || item.type == TR1ItemId::ShotgunSprite
                     || item.type == TR1ItemId::MagnumsSprite
                     || item.type == TR1ItemId::UzisSprite
                     || item.type == TR1ItemId::PistolAmmoSprite
                     || item.type == TR1ItemId::ShotgunAmmoSprite
                     || item.type == TR1ItemId::MagnumAmmoSprite
                     || item.type == TR1ItemId::UziAmmoSprite
                     || item.type == TR1ItemId::ExplosiveSprite
                     || item.type == TR1ItemId::SmallMedipackSprite
                     || item.type == TR1ItemId::LargeMedipackSprite
                     || item.type == TR1ItemId::ScionPiece2
                     || item.type == TR1ItemId::LeadBarSprite )
            {
                node = std::make_shared<items::PickupItem>( this,
                                                            std::string( "sprite(type:" )
                                                            + toString( item.type ) + ")",
                                                            room,
                                                            item,
                                                            sprite,
                                                            m_spriteMaterial );
            }
            else
            {
                BOOST_LOG_TRIVIAL( warning ) << "Unimplemented item " << toString( item.type );
                node = std::make_shared<items::SpriteItemNode>( this,
                                                                std::string( "sprite(type:" )
                                                                + toString( item.type ) + ")",
                                                                room,
                                                                item,
                                                                true,
                                                                sprite,
                                                                m_spriteMaterial );
            }

            m_itemNodes.emplace( std::make_pair( id, node ) );
            continue;
        }

        BOOST_LOG_TRIVIAL( error ) << "Failed to find an appropriate animated model for item " << id << "/type "
                                   << int( item.type );
    }

    return lara;
}

void Engine::setUpRendering()
{
    m_inputHandler = std::make_unique<InputHandler>( game->getWindow() );

    for( auto& sprite : m_level->m_sprites )
    {
        sprite.texture = m_level->m_textures.at( sprite.texture_id.get() ).texture;
        sprite.image = m_level->m_textures[sprite.texture_id.get()].image;
    }

    m_textureAnimator = std::make_shared<render::TextureAnimator>( m_level->m_animatedTextures,
                                                                   m_level->m_textureProxies, m_level->m_textures );

    const auto texturedShader = gameplay::ShaderProgram::createFromFile( "shaders/textured_2.vert",
                                                                         "shaders/textured_2.frag" );
    const auto materials = createMaterials( texturedShader );

    const auto colorMaterial = std::make_shared<gameplay::Material>( "shaders/colored_2.vert",
                                                                     "shaders/colored_2.frag" );
    colorMaterial->getParameter( "u_modelMatrix" )->bindModelMatrix();
    colorMaterial->getParameter( "u_modelViewMatrix" )->bindModelViewMatrix();
    colorMaterial->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    BOOST_ASSERT( m_spriteMaterial == nullptr );
    m_spriteMaterial = std::make_shared<gameplay::Material>( "shaders/textured_2.vert", "shaders/textured_2.frag" );
    m_spriteMaterial->getRenderState().setCullFace( false );

    m_spriteMaterial->getParameter( "u_modelMatrix" )->bindModelMatrix();
    m_spriteMaterial->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    m_spriteMaterial->getParameter( "u_baseLightDiff" )->set( 0.0f );
    m_spriteMaterial->getParameter( "u_lightPosition" )->set( glm::vec3{std::numeric_limits<float>::quiet_NaN()} );

    for( auto& mesh : m_level->m_meshes )
    {
        m_models.emplace_back(
                mesh.createModel( m_level->m_textureProxies, materials, colorMaterial, *m_level->m_palette,
                                  *m_textureAnimator ) );
    }

    for( auto idx : m_level->m_meshIndices )
    {
        Expects( idx < m_models.size() );
        m_modelsDirect.emplace_back( m_models[idx] );
        m_meshesDirect.emplace_back( &m_level->m_meshes[idx] );
    }

    for( const std::unique_ptr<loader::file::SkeletalModelType>& model : m_level->m_animatedModels
                                                                         | boost::adaptors::map_values )
    {
        if( model->nMeshes > 0 )
        {
            model->models = make_span( &model->mesh_base_index.checkedFrom( m_modelsDirect ), model->nMeshes );
            model->meshes = make_span( &model->mesh_base_index.checkedFrom( m_meshesDirect ), model->nMeshes );
        }
    }

    game->getScene()->setActiveCamera(
            std::make_shared<gameplay::Camera>( glm::radians( 80.0f ), game->getAspectRatio(), 10.0f, 20480.0f ) );

    const auto waterTexturedShader = gameplay::ShaderProgram::createFromFile( "shaders/textured_2.vert",
                                                                              "shaders/textured_2.frag",
                                                                              {"WATER"} );
    auto waterMaterials = createMaterials( waterTexturedShader );
    for( const auto& m : waterMaterials | boost::adaptors::map_values )
    {
        m->getParameter( "u_time" )->bind(
                [this](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>( game->getGameTime() );
                    uniform.set( gsl::narrow_cast<float>( now.time_since_epoch().count() ) );
                }
        );
    }

    for( size_t i = 0; i < m_level->m_rooms.size(); ++i )
    {
        m_level->m_rooms[i].createSceneNode( i, *m_level, materials, waterMaterials, m_models, *m_textureAnimator,
                                             m_spriteMaterial );
        game->getScene()->addNode( m_level->m_rooms[i].node );
    }

    m_lara = createItems();
    if( m_lara == nullptr )
    {
        m_cameraController = std::make_unique<CameraController>(
                this,
                game->getScene()->getActiveCamera(),
                true );

        for( const auto& item : m_level->m_items )
        {
            if( item.type == TR1ItemId::CutsceneActor1 )
            {
                m_cameraController->setPosition( item.position );
            }
        }
    }
    else
    {
        m_cameraController = std::make_unique<CameraController>(
                this,
                game->getScene()->getActiveCamera() );
    }

    m_soundEngine.setListener( m_cameraController.get() );

    for( loader::file::SoundSource& src : m_level->m_soundSources )
    {
        auto handle = playSound( src.sound_id, &src );
        handle->setLooping( true );
    }
}

gsl::not_null<const loader::file::Room*>
Engine::findRoomForPosition(const core::TRVec& position, gsl::not_null<const loader::file::Room*> room) const
{
    const loader::file::Sector* sector;
    while( true )
    {
        sector = room->findFloorSectorWithClampedIndex(
                (position.X - room->position.X) / core::SectorSize,
                (position.Z - room->position.Z) / core::SectorSize );
        Expects( sector != nullptr );
        if( sector->portalTarget == nullptr )
        {
            break;
        }

        room = sector->portalTarget;
    }

    Expects( sector != nullptr );
    if( sector->floorHeight > position.Y )
    {
        while( sector->ceilingHeight > position.Y && sector->roomAbove != nullptr )
        {
            room = sector->roomAbove;
            sector = room->getSectorByAbsolutePosition( position );
            Expects( sector != nullptr );
        }
    }
    else
    {
        while( sector->floorHeight <= position.Y && sector->roomBelow != nullptr )
        {
            room = sector->roomBelow;
            sector = room->getSectorByAbsolutePosition( position );
            Expects( sector != nullptr );
        }
    }

    return room;
}

std::shared_ptr<items::ItemNode> Engine::getItem(const uint16_t id) const
{
    const auto it = m_itemNodes.find( id );
    if( it == m_itemNodes.end() )
        return nullptr;

    return it->second.get();
}

void Engine::drawBars(const gsl::not_null<gameplay::Game*>& game,
                      const gsl::not_null<std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>>>& image) const
{
    if( m_lara->isInWater() )
    {
        const auto x0 = gsl::narrow<GLint>( game->getViewport().width - 110 );

        for( int i = 7; i <= 13; ++i )
            image->line( x0 - 1, i, x0 + 101, i, m_level->m_palette->colors[0].toTextureColor() );
        image->line( x0 - 2, 14, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor() );
        image->line( x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor() );
        image->line( x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[19].toTextureColor() );
        image->line( x0 - 2, 6, x0 - 2, 14, m_level->m_palette->colors[19].toTextureColor() );

        const int p = util::clamp( m_lara->getAir() * 100 / core::LaraAir, 0, 100 );
        if( p > 0 )
        {
            image->line( x0, 8, x0 + p, 8, m_level->m_palette->colors[32].toTextureColor() );
            image->line( x0, 9, x0 + p, 9, m_level->m_palette->colors[41].toTextureColor() );
            image->line( x0, 10, x0 + p, 10, m_level->m_palette->colors[32].toTextureColor() );
            image->line( x0, 11, x0 + p, 11, m_level->m_palette->colors[19].toTextureColor() );
            image->line( x0, 12, x0 + p, 12, m_level->m_palette->colors[21].toTextureColor() );
        }
    }

    const int x0 = 8;
    for( int i = 7; i <= 13; ++i )
        image->line( x0 - 1, i, x0 + 101, i, m_level->m_palette->colors[0].toTextureColor() );
    image->line( x0 - 2, 14, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor() );
    image->line( x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[17].toTextureColor() );
    image->line( x0 + 102, 6, x0 + 102, 14, m_level->m_palette->colors[19].toTextureColor() );
    image->line( x0 - 2, 6, x0 - 2, 14, m_level->m_palette->colors[19].toTextureColor() );

    const int p = util::clamp( m_lara->m_state.health * 100 / core::LaraHealth, 0, 100 );
    if( p > 0 )
    {
        image->line( x0, 8, x0 + p, 8, m_level->m_palette->colors[8].toTextureColor() );
        image->line( x0, 9, x0 + p, 9, m_level->m_palette->colors[11].toTextureColor() );
        image->line( x0, 10, x0 + p, 10, m_level->m_palette->colors[8].toTextureColor() );
        image->line( x0, 11, x0 + p, 11, m_level->m_palette->colors[6].toTextureColor() );
        image->line( x0, 12, x0 + p, 12, m_level->m_palette->colors[24].toTextureColor() );
    }
}

void Engine::triggerCdTrack(TR1TrackId trackId,
                            const floordata::ActivationState& activationRequest,
                            const floordata::SequenceCondition triggerType)
{
    if( trackId >= TR1TrackId::Sentinel )
        return;

    if( trackId < TR1TrackId::LaraTalk2 )
    {
        // 1..27
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk2 )
    {
        // 28
        if( m_cdTrackActivationStates[trackId].isOneshot()
            && m_lara->getCurrentAnimState() == LaraStateId::JumpUp )
        {
            trackId = TR1TrackId::LaraTalk3;
        }
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId < TR1TrackId::LaraTalk15 )
    {
        // 29..40
        if( trackId != TR1TrackId::LaraTalk11 )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk15 )
    {
        // 41
        if( m_lara->getCurrentAnimState() == LaraStateId::Hang )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk16 )
    {
        // 42
        if( m_lara->getCurrentAnimState() == LaraStateId::Hang )
            triggerNormalCdTrack( TR1TrackId::LaraTalk17, activationRequest, triggerType );
        else
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId < TR1TrackId::LaraTalk23 )
    {
        // 43..48
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk23 )
    {
        // 49
        if( m_lara->getCurrentAnimState() == LaraStateId::OnWaterStop )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == TR1TrackId::LaraTalk24 )
    {
        // LaraTalk24 "Right. Now I better take off these wet clothes"
        if( m_cdTrackActivationStates[trackId].isOneshot() )
        {
            if( ++m_cdTrack50time == 120 )
            {
                m_levelFinished = true;
                m_cdTrack50time = 0;
                triggerNormalCdTrack( trackId, activationRequest, triggerType );
            }
        }
        else if( m_lara->getCurrentAnimState() == LaraStateId::OnWaterExit )
        {
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
        }
    }
    else
    {
        // 51..64
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
}

void Engine::triggerNormalCdTrack(const TR1TrackId trackId,
                                  const floordata::ActivationState& activationRequest,
                                  const floordata::SequenceCondition triggerType)
{
    if( trackId >= TR1TrackId::Sentinel )
        return;

    if( m_cdTrackActivationStates[trackId].isOneshot() )
        return;

    if( triggerType == floordata::SequenceCondition::ItemActivated )
        m_cdTrackActivationStates[trackId] ^= activationRequest.getActivationSet();
    else if( triggerType == floordata::SequenceCondition::LaraOnGroundInverted )
        m_cdTrackActivationStates[trackId] &= ~activationRequest.getActivationSet();
    else
        m_cdTrackActivationStates[trackId] |= activationRequest.getActivationSet();

    if( !m_cdTrackActivationStates[trackId].isFullyActivated() )
    {
        playStopCdTrack( trackId, true );
        return;
    }

    if( activationRequest.isOneshot() )
        m_cdTrackActivationStates[trackId].setOneshot( true );

    if( !m_currentTrack.is_initialized() || *m_currentTrack != trackId )
        playStopCdTrack( trackId, false );
}

void Engine::playStopCdTrack(const TR1TrackId trackId, bool stop)
{
    const sol::table trackInfo = m_scriptEngine["getTrackInfo"].call( trackId );

    if( !trackInfo )
        return;

    const audio::TrackType trackType = trackInfo["type"];

    switch( trackType )
    {
        case audio::TrackType::AmbientEffect:
            if( !stop )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play effect "
                                           << toString( static_cast<TR1SoundId>(trackInfo["id"]) );
                playSound( static_cast<TR1SoundId>(trackInfo["id"]), nullptr );
            }
            else
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop effect "
                                           << toString( static_cast<TR1SoundId>(trackInfo["id"]) );
                stopSound( static_cast<TR1SoundId>(trackInfo["id"]), nullptr );
            }
            break;
        case audio::TrackType::LaraTalk:
            if( !stop )
            {
                const auto sfxId = static_cast<TR1SoundId>(trackInfo["id"]);

                if( !m_currentLaraTalk.is_initialized() || *m_currentLaraTalk != sfxId )
                {
                    BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play lara talk " << toString( sfxId );

                    if( m_currentLaraTalk.is_initialized() )
                        stopSound( *m_currentLaraTalk, &m_lara->m_state );

                    m_lara->playSoundEffect( sfxId );
                    m_currentLaraTalk = sfxId;
                }
            }
            else
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop lara talk "
                                           << toString( static_cast<TR1SoundId>(trackInfo["id"]) );
                stopSound( static_cast<TR1SoundId>(trackInfo["id"]), &m_lara->m_state );
                m_currentLaraTalk.reset();
            }
            break;
        case audio::TrackType::Ambient:
            if( !stop )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play ambient " << static_cast<size_t>(trackInfo["id"]);
                m_ambientStream = playStream( trackInfo["id"] ).get();
                m_ambientStream.lock()->setLooping( true );
                if( isPlaying( m_interceptStream ) )
                    m_ambientStream.lock()->getSource().lock()->pause();
                m_currentTrack = trackId;
            }
            else if( const auto str = m_ambientStream.lock() )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop ambient " << static_cast<size_t>(trackInfo["id"]);
                m_soundEngine.getDevice().removeStream( str );
                m_currentTrack.reset();
            }
            break;
        case audio::TrackType::Interception:
            if( !stop )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - play interception "
                                           << static_cast<size_t>(trackInfo["id"]);
                if( const auto str = m_interceptStream.lock() )
                    m_soundEngine.getDevice().removeStream( str );
                if( const auto str = m_ambientStream.lock() )
                    str->getSource().lock()->pause();
                m_interceptStream = playStream( trackInfo["id"] ).get();
                m_interceptStream.lock()->setLooping( false );
                m_currentTrack = trackId;
            }
            else if( const auto str = m_interceptStream.lock() )
            {
                BOOST_LOG_TRIVIAL( debug ) << "playStopCdTrack - stop interception "
                                           << static_cast<size_t>(trackInfo["id"]);
                m_soundEngine.getDevice().removeStream( str );
                if( const auto amb = m_ambientStream.lock() )
                    amb->play();
                m_currentTrack.reset();
            }
            break;
    }
}

gsl::not_null<std::shared_ptr<audio::Stream>> Engine::playStream(size_t trackId)
{
    static constexpr size_t DefaultBufferSize = 8192;
    static constexpr size_t DefaultBufferCount = 4;

    std::shared_ptr<audio::Stream> result;
    if( boost::filesystem::is_regular_file( "data/tr1/audio/CDAUDIO.WAD" ) )
        result = m_soundEngine.getDevice().createStream(
                std::make_unique<audio::WadStreamSource>( "data/tr1/audio/CDAUDIO.WAD", trackId ),
                DefaultBufferSize,
                DefaultBufferCount );
    else
        result = m_soundEngine.getDevice().createStream(
                std::make_unique<audio::SndfileStreamSource>(
                        (boost::format( "data/tr1/audio/%03d.ogg" ) % trackId).str() ),
                DefaultBufferSize,
                DefaultBufferCount );

    result->setGain( 0.8f );
    return result;
}

void Engine::useAlternativeLaraAppearance(const bool withHead)
{
    const auto& base = *findAnimatedModelForType( TR1ItemId::Lara );
    BOOST_ASSERT( gsl::narrow<size_t>( base.models.size() ) == m_lara->getNode()->getChildren().size() );

    const auto& alternate = *findAnimatedModelForType( TR1ItemId::AlternativeLara );
    BOOST_ASSERT( gsl::narrow<size_t>( alternate.models.size() ) == m_lara->getNode()->getChildren().size() );

    for( size_t i = 0; i < m_lara->getNode()->getChildren().size(); ++i )
        m_lara->getNode()->getChild( i )->setDrawable( alternate.models[i].get() );

    if( !withHead )
        m_lara->getNode()->getChild( 14 )->setDrawable( base.models[14].get() );
}

void Engine::dinoStompEffect(items::ItemNode& node)
{
    const auto d = node.m_state.position.position.toRenderSystem() - m_cameraController->getPosition();
    const auto absD = glm::abs( d );

    static constexpr auto MaxD = (16 * core::SectorSize).get_as<float>();
    if( absD.x > MaxD || absD.y > MaxD || absD.z > MaxD )
        return;

    const auto x = (100_len).retype_as<float>() * (1 - glm::length2( d ) / util::square( MaxD ));
    m_cameraController->setBounce( x.retype_as<core::Length>() );
}

void Engine::turn180Effect(items::ItemNode& node)
{
    node.m_state.rotation.Y += 180_deg;
}

void Engine::laraNormalEffect()
{
    Expects( m_lara != nullptr );
    m_lara->setCurrentAnimState( LaraStateId::Stop );
    m_lara->setRequiredAnimState( LaraStateId::Unknown12 );
    m_lara->m_state.anim = &m_level->m_animations[static_cast<int>(AnimationId::STAY_SOLID)];
    m_lara->m_state.frame_number = 185_frame;
    m_cameraController->setMode( CameraMode::Chase );
    m_cameraController->getCamera()->setFieldOfView( glm::radians( 80.0f ) );
}

void Engine::laraBubblesEffect(items::ItemNode& node)
{
    const auto modelNode = dynamic_cast<items::ModelItemNode*>(&node);
    if( modelNode == nullptr )
        return;

    auto bubbleCount = util::rand15( 12 );
    if( bubbleCount == 0 )
        return;

    node.playSoundEffect( TR1SoundId::LaraUnderwaterGurgle );

    const auto itemSpheres = modelNode->getSkeleton()->getBoneCollisionSpheres(
            node.m_state,
            *modelNode->getSkeleton()->getInterpolationInfo( modelNode->m_state ).getNearestFrame(),
            nullptr );

    const auto position = core::TRVec{
            glm::vec3{translate( itemSpheres.at( 14 ).m, core::TRVec{0_len, 0_len, 50_len}.toRenderSystem() )[3]}};

    while( bubbleCount-- > 0 )
    {
        auto particle = std::make_shared<BubbleParticle>(
                core::RoomBoundPosition{node.m_state.position.room, position}, *this );
        setParent( particle, node.m_state.position.room->node );
        m_particles.emplace_back( particle );
    }
}

void Engine::finishLevelEffect()
{
    m_levelFinished = true;
}

void Engine::earthquakeEffect()
{
    switch( m_effectTimer.get() )
    {
        case 0:
            playSound( TR1SoundId::Explosion1, nullptr );
            m_cameraController->setBounce( -250_len );
            break;
        case 3:
            playSound( TR1SoundId::RollingBall, nullptr );
            break;
        case 35:
            playSound( TR1SoundId::Explosion1, nullptr );
            break;
        case 20:
        case 50:
        case 70:
            playSound( TR1SoundId::TRexFootstep, nullptr );
            break;
        default:
            // silence compiler
            break;
    }

    m_effectTimer += 1_frame;
    if( m_effectTimer == 105_frame )
    {
        m_activeEffect.reset();
    }
}

void Engine::floodEffect()
{
    if( m_effectTimer <= 120_frame )
    {
        auto pos = m_lara->m_state.position.position;
        core::Frame mul = 0_frame;
        if( m_effectTimer >= 30_frame )
        {
            mul = m_effectTimer - 30_frame;
        }
        else
        {
            mul = 30_frame - m_effectTimer;
        }
        pos.Y = 100_len * mul / 1_frame + m_cameraController->getCenter().position.Y;
        playSound( TR1SoundId::WaterFlow3, pos.toRenderSystem() );
    }
    else
    {
        m_activeEffect.reset();
    }
    m_effectTimer += 1_frame;
}

void Engine::chandelierEffect()
{
    playSound( TR1SoundId::GlassyFlow, nullptr );
    m_activeEffect.reset();
}

void Engine::raisingBlockEffect()
{
    m_effectTimer += 1_frame;
    if( m_effectTimer == 5_frame )
    {
        playSound( TR1SoundId::Clank, nullptr );
        m_activeEffect.reset();
    }
}

void Engine::stairsToSlopeEffect()
{
    if( m_effectTimer <= 120_frame )
    {
        if( m_effectTimer == 0_frame )
        {
            playSound( TR1SoundId::HeavyDoorSlam, nullptr );
        }
        auto pos = m_cameraController->getCenter().position;
        pos.Y += 100_spd * m_effectTimer;
        playSound( TR1SoundId::FlowingAir, pos.toRenderSystem() );
    }
    else
    {
        m_activeEffect.reset();
    }
    m_effectTimer += 1_frame;
}

void Engine::sandEffect()
{
    if( m_effectTimer <= 120_frame )
    {
        playSound( TR1SoundId::LowHum, nullptr );
    }
    else
    {
        m_activeEffect.reset();
    }
    m_effectTimer += 1_frame;
}

void Engine::explosionEffect()
{
    playSound( TR1SoundId::LowPitchedSettling, nullptr );
    m_cameraController->setBounce( -75_len );
    m_activeEffect.reset();
}

void Engine::laraHandsFreeEffect()
{
    m_lara->setHandStatus( HandStatus::None );
}

void Engine::flipMapEffect()
{
    swapAllRooms();
}

void Engine::unholsterRightGunEffect(items::ItemNode& node)
{
    const auto& src = *findAnimatedModelForType( TR1ItemId::LaraPistolsAnim );
    BOOST_ASSERT( gsl::narrow<size_t>( src.models.size() ) == node.getNode()->getChildren().size() );
    node.getNode()->getChild( 10 )->setDrawable( src.models[10].get() );
}

void Engine::chainBlockEffect()
{
    if( m_effectTimer == 0_frame )
    {
        playSound( TR1SoundId::SecretFound, nullptr );
    }
    m_effectTimer += 1_frame;
    if( m_effectTimer == 55_frame )
    {
        playSound( TR1SoundId::LaraFallIntoWater, nullptr );
        m_activeEffect.reset();
    }
}

void Engine::flickerEffect()
{
    if( m_effectTimer == 90_frame || m_effectTimer == 92_frame || m_effectTimer == 105_frame
        || m_effectTimer == 107_frame )
    {
        swapAllRooms();
    }
    else if( m_effectTimer > 125_frame )
    {
        swapAllRooms();
        m_activeEffect.reset();
    }
    m_effectTimer += 1_frame;
}

void Engine::swapWithAlternate(loader::file::Room& orig, loader::file::Room& alternate)
{
    // find any blocks in the original room and un-patch the floor heights

    for( const auto& item : m_itemNodes | boost::adaptors::map_values )
    {
        if( item->m_state.position.room != &orig )
            continue;

        if( const auto tmp = std::dynamic_pointer_cast<items::Block>( item.get() ) )
        {
            loader::file::Room::patchHeightsForBlock( *tmp, core::SectorSize );
        }
        else if( const auto tmp = std::dynamic_pointer_cast<items::TallBlock>( item.get() ) )
        {
            loader::file::Room::patchHeightsForBlock( *tmp, core::SectorSize * 2 );
        }
    }

    // now swap the rooms and patch the alternate room ids
    std::swap( orig, alternate );
    orig.alternateRoom = alternate.alternateRoom;
    alternate.alternateRoom = int16_t( -1 );

    // patch heights in the new room, and swap item ownerships.
    // note that this is exactly the same code as above,
    // except for the heights.
    for( const auto& item : m_itemNodes | boost::adaptors::map_values )
    {
        if( item->m_state.position.room == &orig )
        {
            // although this seems contradictory, remember the nodes have been swapped above
            setParent( item->getNode(), orig.node );
        }
        else if( item->m_state.position.room == &alternate )
        {
            setParent( item->getNode(), alternate.node );
            continue;
        }
        else
        {
            continue;
        }

        if( const auto tmp = std::dynamic_pointer_cast<items::Block>( item.get() ) )
        {
            loader::file::Room::patchHeightsForBlock( *tmp, -core::SectorSize );
        }
        else if( const auto tmp = std::dynamic_pointer_cast<items::TallBlock>( item.get() ) )
        {
            loader::file::Room::patchHeightsForBlock( *tmp, -core::SectorSize * 2 );
        }
    }

    for( const auto& item : m_dynamicItems )
    {
        if( item->m_state.position.room == &orig )
        {
            setParent( item->getNode(), orig.node );
        }
        else if( item->m_state.position.room == &alternate )
        {
            setParent( item->getNode(), alternate.node );
        }
    }
}

void Engine::addInventoryItem(const TR1ItemId id, const size_t quantity)
{
    BOOST_LOG_TRIVIAL( debug ) << "Item " << toString( id ) << " added to inventory";

    switch( id )
    {
        case TR1ItemId::PistolsSprite:
        case TR1ItemId::Pistols:
            m_inventory[TR1ItemId::Pistols] += quantity;
            break;
        case TR1ItemId::ShotgunSprite:
        case TR1ItemId::Shotgun:
            if( const auto clips = countInventoryItem( TR1ItemId::ShotgunAmmoSprite ) )
            {
                takeInventoryItem( TR1ItemId::ShotgunAmmoSprite, clips );
                m_lara->shotgunAmmo.ammo += 12 * clips;
            }
            m_lara->shotgunAmmo.ammo += 12 * quantity;
            // TODO replaceItems( ShotgunSprite, ShotgunAmmoSprite );
            m_inventory[TR1ItemId::Shotgun] = 1;
            break;
        case TR1ItemId::MagnumsSprite:
        case TR1ItemId::Magnums:
            if( const auto clips = countInventoryItem( TR1ItemId::MagnumAmmoSprite ) )
            {
                takeInventoryItem( TR1ItemId::MagnumAmmoSprite, clips );
                m_lara->revolverAmmo.ammo += 50 * clips;
            }
            m_lara->revolverAmmo.ammo += 50 * quantity;
            // TODO replaceItems( MagnumsSprite, MagnumAmmoSprite );
            m_inventory[TR1ItemId::Magnums] = 1;
            break;
        case TR1ItemId::UzisSprite:
        case TR1ItemId::Uzis:
            if( const auto clips = countInventoryItem( TR1ItemId::UziAmmoSprite ) )
            {
                takeInventoryItem( TR1ItemId::UziAmmoSprite, clips );
                m_lara->uziAmmo.ammo += 100 * clips;
            }
            m_lara->uziAmmo.ammo += 100 * quantity;
            // TODO replaceItems( UzisSprite, UziAmmoSprite );
            m_inventory[TR1ItemId::Uzis] = 1;
            break;
        case TR1ItemId::ShotgunAmmoSprite:
        case TR1ItemId::ShotgunAmmo:
            if( countInventoryItem( TR1ItemId::ShotgunSprite ) > 0 )
                m_lara->shotgunAmmo.ammo += 12;
            else
                m_inventory[TR1ItemId::ShotgunAmmo] += quantity;
            break;
        case TR1ItemId::MagnumAmmoSprite:
        case TR1ItemId::MagnumAmmo:
            if( countInventoryItem( TR1ItemId::MagnumsSprite ) > 0 )
                m_lara->revolverAmmo.ammo += 50;
            else
                m_inventory[TR1ItemId::MagnumAmmo] += quantity;
            break;
        case TR1ItemId::UziAmmoSprite:
        case TR1ItemId::UziAmmo:
            if( countInventoryItem( TR1ItemId::UzisSprite ) > 0 )
                m_lara->uziAmmo.ammo += 100;
            else
                m_inventory[TR1ItemId::UziAmmo] += quantity;
            break;
        case TR1ItemId::SmallMedipackSprite:
        case TR1ItemId::SmallMedipack:
            m_inventory[TR1ItemId::SmallMedipack] += quantity;
            break;
        case TR1ItemId::LargeMedipackSprite:
        case TR1ItemId::LargeMedipack:
            m_inventory[TR1ItemId::LargeMedipack] += quantity;
            break;
        case TR1ItemId::Puzzle1Sprite:
        case TR1ItemId::Puzzle1:
            m_inventory[TR1ItemId::Puzzle1] += quantity;
            break;
        case TR1ItemId::Puzzle2Sprite:
        case TR1ItemId::Puzzle2:
            m_inventory[TR1ItemId::Puzzle2] += quantity;
            break;
        case TR1ItemId::Puzzle3Sprite:
        case TR1ItemId::Puzzle3:
            m_inventory[TR1ItemId::Puzzle3] += quantity;
            break;
        case TR1ItemId::Puzzle4Sprite:
        case TR1ItemId::Puzzle4:
            m_inventory[TR1ItemId::Puzzle4] += quantity;
            break;
        case TR1ItemId::LeadBarSprite:
        case TR1ItemId::LeadBar:
            m_inventory[TR1ItemId::LeadBar] += quantity;
            break;
        case TR1ItemId::Key1Sprite:
        case TR1ItemId::Key1:
            m_inventory[TR1ItemId::Key1] += quantity;
            break;
        case TR1ItemId::Key2Sprite:
        case TR1ItemId::Key2:
            m_inventory[TR1ItemId::Key2] += quantity;
            break;
        case TR1ItemId::Key3Sprite:
        case TR1ItemId::Key3:
            m_inventory[TR1ItemId::Key3] += quantity;
            break;
        case TR1ItemId::Key4Sprite:
        case TR1ItemId::Key4:
            m_inventory[TR1ItemId::Key4] += quantity;
            break;
        case TR1ItemId::Item141:
        case TR1ItemId::Item148:
            m_inventory[TR1ItemId::Item148] += quantity;
            break;
        case TR1ItemId::Item142:
        case TR1ItemId::Item149:
            m_inventory[TR1ItemId::Item149] += quantity;
            break;
        case TR1ItemId::ScionPiece1:
        case TR1ItemId::ScionPiece2:
        case TR1ItemId::ScionPiece5:
            m_inventory[TR1ItemId::ScionPiece5] += quantity;
            break;
        default:
            BOOST_LOG_TRIVIAL( warning ) << "Cannot add item " << toString( id ) << " to inventory";
            return;
    }
}

bool Engine::tryUseInventoryItem(const TR1ItemId id)
{
    if( id == TR1ItemId::Shotgun || id == TR1ItemId::ShotgunSprite )
    {
        if( countInventoryItem( TR1ItemId::Shotgun ) == 0 )
            return false;

        m_lara->requestedGunType = LaraNode::WeaponId::Shotgun;
        if( m_lara->getHandStatus() == HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = LaraNode::WeaponId::None;
        }
    }
    else if( id == TR1ItemId::Pistols || id == TR1ItemId::PistolsSprite )
    {
        if( countInventoryItem( TR1ItemId::Pistols ) == 0 )
            return false;

        m_lara->requestedGunType = LaraNode::WeaponId::Pistols;
        if( m_lara->getHandStatus() == HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = LaraNode::WeaponId::None;
        }
    }
    else if( id == TR1ItemId::Magnums || id == TR1ItemId::MagnumsSprite )
    {
        if( countInventoryItem( TR1ItemId::Magnums ) == 0 )
            return false;

        m_lara->requestedGunType = LaraNode::WeaponId::AutoPistols;
        if( m_lara->getHandStatus() == HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = LaraNode::WeaponId::None;
        }
    }
    else if( id == TR1ItemId::Uzis || id == TR1ItemId::UzisSprite )
    {
        if( countInventoryItem( TR1ItemId::Uzis ) == 0 )
            return false;

        m_lara->requestedGunType = LaraNode::WeaponId::Uzi;
        if( m_lara->getHandStatus() == HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = LaraNode::WeaponId::None;
        }
    }
    else if( id == TR1ItemId::LargeMedipack || id == TR1ItemId::LargeMedipackSprite )
    {
        if( countInventoryItem( TR1ItemId::LargeMedipack ) == 0 )
            return false;

        if( m_lara->m_state.health <= 0_hp || m_lara->m_state.health >= core::LaraHealth )
        {
            return false;
        }

        m_lara->m_state.health += 1000_hp;
        if( m_lara->m_state.health > core::LaraHealth )
        {
            m_lara->m_state.health = core::LaraHealth;
        }
        takeInventoryItem( TR1ItemId::LargeMedipackSprite );
        playSound( TR1SoundId::LaraSigh, &m_lara->m_state );
    }
    else if( id == TR1ItemId::SmallMedipack || id == TR1ItemId::SmallMedipackSprite )
    {
        if( countInventoryItem( TR1ItemId::SmallMedipack ) == 0 )
            return false;

        if( m_lara->m_state.health <= 0_hp || m_lara->m_state.health >= core::LaraHealth )
        {
            return false;
        }

        m_lara->m_state.health += 500_hp;
        if( m_lara->m_state.health > core::LaraHealth )
        {
            m_lara->m_state.health = core::LaraHealth;
        }
        takeInventoryItem( TR1ItemId::SmallMedipackSprite );
        playSound( TR1SoundId::LaraSigh, &m_lara->m_state );
    }

    return true;
}

void Engine::animateUV()
{
    static constexpr auto UVAnimTime = 10;

    ++m_uvAnimTime;
    if( m_uvAnimTime >= UVAnimTime )
    {
        m_textureAnimator->updateCoordinates( m_level->m_textureProxies );
        m_uvAnimTime -= UVAnimTime;
    }
}

YAML::Node Engine::save() const
{
    YAML::Node result;

    YAML::Node inventory;

    const auto addInventory = [&](const TR1ItemId id) {
        inventory[toString( id )] = countInventoryItem( id );
    };

    addInventory( TR1ItemId::Pistols );
    addInventory( TR1ItemId::Magnums );
    addInventory( TR1ItemId::MagnumAmmo );
    addInventory( TR1ItemId::Uzis );
    addInventory( TR1ItemId::UziAmmo );
    addInventory( TR1ItemId::Shotgun );
    addInventory( TR1ItemId::ShotgunAmmo );
    addInventory( TR1ItemId::SmallMedipack );
    addInventory( TR1ItemId::LargeMedipack );
    addInventory( TR1ItemId::ScionPiece1 );
    addInventory( TR1ItemId::Item141 );
    addInventory( TR1ItemId::Item142 );
    addInventory( TR1ItemId::Puzzle1 );
    addInventory( TR1ItemId::Puzzle2 );
    addInventory( TR1ItemId::Puzzle3 );
    addInventory( TR1ItemId::Puzzle4 );
    addInventory( TR1ItemId::Key1 );
    addInventory( TR1ItemId::Key2 );
    addInventory( TR1ItemId::Key3 );
    addInventory( TR1ItemId::Key4 );
    addInventory( TR1ItemId::LeadBar );

    result["inventory"] = inventory;

    YAML::Node flipStatus;
    for( const auto& state : mapFlipActivationStates )
    {
        flipStatus.push_back( state.save() );
    }
    result["flipStatus"] = flipStatus;

    YAML::Node cameraFlags;
    for( const auto& camera : m_level->m_cameras )
    {
        cameraFlags.push_back( camera.flags );
    }
    result["cameraFlags"] = cameraFlags;

    if( m_activeEffect.is_initialized() )
        result["flipEffect"] = *m_activeEffect;
    result["flipEffectTimer"] = m_effectTimer;

    for( const auto& item : m_itemNodes )
    {
        result["items"][item.first] = item.second->save();
    }

    result["camera"] = m_cameraController->save();

    return result;
}

void Engine::load(const YAML::Node& node)
{
    m_inventory.clear();

    for( const auto& entry : node["inventory"] )
        addInventoryItem(
                EnumUtil<TR1ItemId>::fromString( entry.first.as<std::string>() ),
                entry.second.as<size_t>() );

    for( size_t i = 0; i < mapFlipActivationStates.size(); ++i )
    {
        mapFlipActivationStates[i].load( node["flipStatus"][i] );
    }

    if( !node["cameraFlags"].IsSequence() || node["cameraFlags"].size() != m_level->m_cameras.size() )
        BOOST_THROW_EXCEPTION( std::domain_error( "Camera flag sequence is invalid" ) );

    for( size_t i = 0; i < m_level->m_cameras.size(); ++i )
        m_level->m_cameras[i].flags = node["cameraFlags"][i].as<uint16_t>();

    if( !node["flipEffect"].IsDefined() )
        m_activeEffect.reset();
    else
        m_activeEffect = node["flipEffect"].as<size_t>();

    m_effectTimer = node["flipEffectTimer"].as<core::Frame>();

    for( const auto& item : m_itemNodes )
    {
        item.second->load( node["items"][item.first] );
    }

    m_cameraController->load( node["camera"] );
}

std::shared_ptr<audio::SourceHandle> Engine::playSound(const TR1SoundId id, audio::Emitter* emitter)
{
    Expects( static_cast<size_t>(id) < m_level->m_soundmap.size() );
    const auto snd = m_level->m_soundmap[static_cast<size_t>(id)];
    if( snd < 0 )
    {
        BOOST_LOG_TRIVIAL( warning ) << "No mapped sound for id " << toString( id );
        return nullptr;
    }

    BOOST_ASSERT( snd >= 0 && static_cast<size_t>(snd) < m_level->m_soundDetails.size() );
    const loader::file::SoundDetails& details = m_level->m_soundDetails[snd];
    if( details.chance != 0 && util::rand15() > details.chance )
        return nullptr;

    size_t sample = details.sample.get();
    if( details.getSampleCount() > 1 )
        sample += util::rand15( details.getSampleCount() );
    BOOST_ASSERT( sample < m_level->m_sampleIndices.size() );

    float pitch = 1;
    if( details.useRandomPitch() )
        pitch = 0.9f + util::rand15( 0.2f );

    float volume = util::clamp( static_cast<float>(details.volume) / 0x7fff, 0.0f, 1.0f );
    if( details.useRandomVolume() )
        volume -= util::rand15( 0.25f );
    if( volume <= 0 )
        return nullptr;

    std::shared_ptr<audio::SourceHandle> handle;
    if( details.getPlaybackType( loader::file::level::Engine::TR1 ) == loader::file::PlaybackType::Looping )
    {
        auto handles = m_soundEngine.getSourcesForBuffer( emitter, sample );
        if( handles.empty() )
        {
            BOOST_LOG_TRIVIAL( debug ) << "Play looping sound " << toString( id );
            handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
            handle->setLooping( true );
            handle->play();
        }
        else
        {
            BOOST_ASSERT( handles.size() == 1 );
        }
    }
    else if( details.getPlaybackType( loader::file::level::Engine::TR1 ) == loader::file::PlaybackType::Restart )
    {
        auto handles = m_soundEngine.getSourcesForBuffer( emitter, sample );
        if( !handles.empty() )
        {
            BOOST_ASSERT( handles.size() == 1 );
            BOOST_LOG_TRIVIAL( debug ) << "Update restarting sound " << toString( id );
            handle = handles[0];
            handle->setPitch( pitch );
            handle->setGain( volume );
            if( emitter != nullptr )
                handle->setPosition( emitter->getPosition() );
            handle->play();
        }
        else
        {
            BOOST_LOG_TRIVIAL( debug ) << "Play restarting sound " << toString( id );
            handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
        }
    }
    else if( details.getPlaybackType( loader::file::level::Engine::TR1 ) == loader::file::PlaybackType::Wait )
    {
        auto handles = m_soundEngine.getSourcesForBuffer( emitter, sample );
        if( handles.empty() )
        {
            BOOST_LOG_TRIVIAL( debug ) << "Play non-playing sound " << toString( id );
            handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
        }
        else
        {
            BOOST_ASSERT( handles.size() == 1 );
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL( debug ) << "Default play mode - playing sound " << toString( id );
        handle = m_soundEngine.playBuffer( sample, pitch, volume, emitter );
    }

    return handle;
}

void Engine::stopSound(const TR1SoundId soundId, audio::Emitter* emitter)
{
    BOOST_ASSERT( static_cast<size_t>(soundId) < m_level->m_soundmap.size() );
    const auto& details = m_level->m_soundDetails[m_level->m_soundmap[static_cast<size_t>(soundId)]];
    const size_t first = details.sample.get();
    const size_t last = first + details.getSampleCount();

    bool anyStopped = false;
    for( size_t i = first; i < last; ++i )
    {
        anyStopped |= m_soundEngine.stopBuffer( i, emitter );
    }

    if( !anyStopped )
        BOOST_LOG_TRIVIAL( debug ) << "Attempting to stop sound " << toString( soundId )
                                   << " (samples " << first << ".." << (last - 1) << ") didn't stop any sample";
    else
        BOOST_LOG_TRIVIAL( debug ) << "Stopped samples of sound " << toString( soundId );
}

std::shared_ptr<items::PickupItem> Engine::createPickup(const TR1ItemId type,
                                                        const gsl::not_null<const loader::file::Room*>& room,
                                                        const core::TRVec& position)
{
    loader::file::Item item;
    item.type = type;
    item.room = uint16_t( -1 );
    item.position = position;
    item.rotation = 0_deg;
    item.darkness = 0;
    item.activationState = 0;

    const auto& spriteSequence = findSpriteSequenceForType( type );
    Expects( spriteSequence != nullptr && !spriteSequence->sprites.empty() );
    const loader::file::Sprite& sprite = spriteSequence->sprites[0];

    auto node = std::make_shared<items::PickupItem>(
            this,
            "pickup",
            room,
            item,
            sprite,
            m_spriteMaterial );

    m_dynamicItems.emplace( node );
    addChild( room->node, node->getNode() );

    return node;
}

void Engine::doGlobalEffect()
{
    if( m_activeEffect.is_initialized() )
        runEffect( *m_activeEffect, nullptr );

    if( m_cameraController->getCurrentRoom()->isWaterRoom() )
    {
        if( isPlaying( m_ambientStream ) )
            m_ambientStream.lock()
                           ->getSource().lock()->setDirectFilter( m_soundEngine.getDevice().getUnderwaterFilter() );

        if( isPlaying( m_interceptStream ) )
            m_interceptStream.lock()
                             ->getSource().lock()->setDirectFilter( m_soundEngine.getDevice().getUnderwaterFilter() );

        if( m_underwaterAmbience.expired() )
        {
            m_underwaterAmbience = playSound( TR1SoundId::UnderwaterAmbience, nullptr );
            m_underwaterAmbience.lock()->setLooping( true );
        }
    }
    else if( !m_underwaterAmbience.expired() )
    {
        if( !m_ambientStream.expired() )
            m_ambientStream.lock()->getSource().lock()->setDirectFilter( nullptr );

        if( isPlaying( m_interceptStream ) )
            m_interceptStream.lock()->getSource().lock()->setDirectFilter( nullptr );

        stopSound( TR1SoundId::UnderwaterAmbience, nullptr );
        m_underwaterAmbience.reset();
    }

    if( !isPlaying( m_interceptStream ) )
    {
        if( const auto str = m_ambientStream.lock() )
            str->play();
    }
}

const loader::file::Animation& Engine::getAnimation(loader::file::AnimationId id) const
{
    return m_level->m_animations.at( static_cast<int>(id) );
}

const std::vector<loader::file::CinematicFrame>& Engine::getCinematicFrames() const
{
    return m_level->m_cinematicFrames;
}

const std::vector<loader::file::Camera>& engine::Engine::getCameras() const
{
    return m_level->m_cameras;
}

const std::vector<int16_t>& Engine::getAnimCommands() const
{
    return m_level->m_animCommands;
}

void Engine::update(const bool godMode)
{
    for( const auto& item : m_itemNodes | boost::adaptors::map_values )
    {
        if( item.get() == m_lara ) // Lara is special and needs to be updated last
            continue;

        if( item->m_isActive )
            item->update();

        item->getNode()->setVisible( item->m_state.triggerState != engine::items::TriggerState::Invisible );
    }

    for( const auto& item : m_dynamicItems )
    {
        if( item->m_isActive )
            item->update();

        item->getNode()->setVisible( item->m_state.triggerState != engine::items::TriggerState::Invisible );
    }

    auto currentParticles = std::move( m_particles );
    for( const auto& particle : currentParticles )
    {
        if( particle->update( *this ) )
        {
            m_particles.emplace_back( particle );
            setParent( particle, particle->pos.room->node );
            particle->updateLight();
        }
        else
        {
            setParent( particle, nullptr );
        }
    }

    if( m_lara != nullptr )
    {
        if( godMode )
            m_lara->m_state.health = core::LaraHealth;
        m_lara->update();
    }

    applyScheduledDeletions();
    animateUV();
}

void Engine::drawDebugInfo(const gsl::not_null<std::shared_ptr<gameplay::gl::Font>>& font, const int fps)
{
    drawText( font, font->getTarget()->getWidth() - 40, font->getTarget()->getHeight() - 20, std::to_string( fps ) );

    if( m_lara != nullptr )
    {
        // position/rotation
        drawText( font, 10, 40, m_lara->m_state.position.room->node->getId() );

        drawText( font, 300, 20,
                  std::to_string( std::lround( m_lara->m_state.rotation.Y.toDegrees() ) ) + " deg" );
        drawText( font, 300, 40, "x=" + m_lara->m_state.position.position.X.toString() );
        drawText( font, 300, 60, "y=" + m_lara->m_state.position.position.Y.toString() );
        drawText( font, 300, 80, "z=" + m_lara->m_state.position.position.Z.toString() );

        // physics
        drawText( font, 300, 100, "grav " + m_lara->m_state.fallspeed.toString() );
        drawText( font, 300, 120, "fwd  " + m_lara->m_state.speed.toString() );

        // animation
        drawText( font, 10, 60,
                  std::string( "current/anim    " ) + toString( m_lara->getCurrentAnimState() ) );
        drawText( font, 10, 100,
                  std::string( "target          " ) + toString( m_lara->getGoalAnimState() ) );
        drawText( font, 10, 120,
                  std::string( "frame           " ) + m_lara->m_state.frame_number.toString() );
    }

    // triggers
    {
        int y = 180;
        for( const auto& item : m_itemNodes | boost::adaptors::map_values )
        {
            if( !item->m_isActive )
                continue;

            drawText( font, 10, y, item->getNode()->getId() );
            switch( item->m_state.triggerState )
            {
                case engine::items::TriggerState::Inactive:
                    drawText( font, 180, y, "inactive" );
                    break;
                case engine::items::TriggerState::Active:
                    drawText( font, 180, y, "active" );
                    break;
                case engine::items::TriggerState::Deactivated:
                    drawText( font, 180, y, "deactivated" );
                    break;
                case engine::items::TriggerState::Invisible:
                    drawText( font, 180, y, "invisible" );
                    break;
            }
            drawText( font, 260, y, item->m_state.timer.toString() );
            y += 20;
        }
    }

    if( m_lara == nullptr )
        return;

#ifndef NDEBUG
    // collision
    drawText( font, 400, 20, boost::lexical_cast<std::string>( "AxisColl: " )
                             + toString( m_lara->lastUsedCollisionInfo.collisionType ) );
    drawText( font, 400, 40, boost::lexical_cast<std::string>( "Current floor:   " )
                             + m_lara->lastUsedCollisionInfo.mid.floorSpace.y.toString() );
    drawText( font, 400, 60, boost::lexical_cast<std::string>( "Current ceiling: " )
                             + m_lara->lastUsedCollisionInfo.mid.ceilingSpace.y.toString() );
    drawText( font, 400, 80, boost::lexical_cast<std::string>( "Front floor:     " )
                             + m_lara->lastUsedCollisionInfo.front.floorSpace.y.toString() );
    drawText( font, 400, 100, boost::lexical_cast<std::string>( "Front ceiling:   " )
                              + m_lara->lastUsedCollisionInfo.front.ceilingSpace.y.toString() );
    drawText( font, 400, 120, boost::lexical_cast<std::string>( "Front/L floor:   " )
                              + m_lara->lastUsedCollisionInfo.frontLeft.floorSpace.y.toString() );
    drawText( font, 400, 140, boost::lexical_cast<std::string>( "Front/L ceiling: " )
                              + m_lara->lastUsedCollisionInfo.frontLeft.ceilingSpace.y.toString() );
    drawText( font, 400, 160, boost::lexical_cast<std::string>( "Front/R floor:   " )
                              + m_lara->lastUsedCollisionInfo.frontRight.floorSpace.y.toString() );
    drawText( font, 400, 180, boost::lexical_cast<std::string>( "Front/R ceiling: " )
                              + m_lara->lastUsedCollisionInfo.frontRight.ceilingSpace.y.toString() );
    drawText( font, 400, 200, boost::lexical_cast<std::string>( "Need bottom:     " )
                              + m_lara->lastUsedCollisionInfo.badPositiveDistance.toString() );
    drawText( font, 400, 220, boost::lexical_cast<std::string>( "Need top:        " )
                              + m_lara->lastUsedCollisionInfo.badNegativeDistance.toString() );
    drawText( font, 400, 240, boost::lexical_cast<std::string>( "Need ceiling:    " )
                              + m_lara->lastUsedCollisionInfo.badCeilingDistance.toString() );
#endif

    // weapons
    drawText( font, 400, 280, std::string( "L.aiming    " ) + (m_lara->leftArm.aiming ? "true" : "false") );
    drawText( font, 400, 300,
              std::string( "L.aim       X=" ) + std::to_string( m_lara->leftArm.aimRotation.X.toDegrees() )
              + ", Y=" + std::to_string( m_lara->leftArm.aimRotation.Y.toDegrees() ) );
    drawText( font, 400, 320, std::string( "R.aiming    " ) + (m_lara->rightArm.aiming ? "true" : "false") );
    drawText( font, 400, 340,
              std::string( "R.aim       X=" ) + std::to_string( m_lara->rightArm.aimRotation.X.toDegrees() )
              + ", Y=" + std::to_string( m_lara->rightArm.aimRotation.Y.toDegrees() ) );
}

void Engine::drawText(const gsl::not_null<std::shared_ptr<gameplay::gl::Font>>& font, const int x, const int y,
                      const std::string& txt, const gameplay::gl::RGBA8& col)
{
    font->drawText( txt, x, y, col.r, col.g, col.b, col.a );
}

Engine::Engine()
        : game{std::make_unique<gameplay::Game>()}
        , splashImage{"splash.png"}
        , abibasFont{std::make_shared<gameplay::gl::Font>( "abibas.ttf", 48 )}
        , m_scriptEngine{createScriptEngine()}
{
    game->initialize();
    game->getScene()->setActiveCamera(
            std::make_shared<gameplay::Camera>( glm::radians( 80.0f ), game->getAspectRatio(), 10.0f, 20480.0f ) );

    scaleSplashImage();

    screenOverlay = std::make_shared<gameplay::ScreenOverlay>( game->getViewport() );

    abibasFont->setTarget( screenOverlay->getImage() );

    drawLoadingScreen( "Booting" );

    try
    {
        m_scriptEngine.safe_script_file( "scripts/main.lua" );
    }
    catch( sol::error& e )
    {
        BOOST_LOG_TRIVIAL( fatal ) << "Failed to load main.lua: " << e.what();
        BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to load main.lua" ) );
    }

    const sol::optional<std::string> glidosPack = m_scriptEngine["getGlidosPack"]();

    std::unique_ptr<loader::trx::Glidos> glidos;
    if( glidosPack && boost::filesystem::is_directory( glidosPack.value() ) )
    {
        drawLoadingScreen( "Loading Glidos texture pack" );
        glidos = std::make_unique<loader::trx::Glidos>( glidosPack.value(),
                                                        [this](const std::string& s) { drawLoadingScreen( s ); } );
    }

    levelInfo = m_scriptEngine["getLevelInfo"]();
    if( !levelInfo.get<std::string>( "video" ).empty() )
        BOOST_THROW_EXCEPTION( std::runtime_error( "Videos not yet supported" ) );

    const auto cutsceneName = levelInfo.get<std::string>( "cutscene" );

    const auto baseName = cutsceneName.empty() ? levelInfo.get<std::string>( "baseName" ) : cutsceneName;
    Expects( !baseName.empty() );
    sol::optional<engine::TR1TrackId> trackToPlay = levelInfo["track"];
    const bool useAlternativeLara = levelInfo.get_or( "useAlternativeLara", false );

    std::map<engine::TR1ItemId, size_t> initInv;

    if( sol::optional<sol::table> tbl = levelInfo["inventory"] )
    {
        for( const auto& kv : *tbl )
            initInv[engine::EnumUtil<engine::TR1ItemId>::fromString( kv.first.as<std::string>() )]
                    += kv.second.as<size_t>();
    }

    if( sol::optional<sol::table> tbl = m_scriptEngine["cheats"]["inventory"] )
    {
        for( const auto& kv : *tbl )
            initInv[engine::EnumUtil<engine::TR1ItemId>::fromString( kv.first.as<std::string>() )]
                    += kv.second.as<size_t>();
    }

    drawLoadingScreen( "Preparing to load " + baseName );

    m_level = loader::file::level::Level::createLoader( "data/tr1/data/" + baseName + ".PHD",
                                                        loader::file::level::Game::Unknown );

    drawLoadingScreen( "Loading " + baseName );

    m_level->loadFileData( m_soundEngine );

    BOOST_LOG_TRIVIAL( info ) << "Loading samples...";

    for( const auto offset : m_level->m_sampleIndices )
    {
        Expects( offset < m_level->m_samplesData.size() );
        m_soundEngine.addWav( &m_level->m_samplesData[offset] );
    }

    for( size_t i = 0; i < m_level->m_textures.size(); ++i )
    {
        if( glidos != nullptr )
            drawLoadingScreen( "Upgrading texture " + std::to_string( i + 1 ) + " of "
                               + std::to_string( m_level->m_textures.size() ) );
        else
            drawLoadingScreen(
                    "Loading texture " + std::to_string( i + 1 ) + " of "
                    + std::to_string( m_level->m_textures.size() ) );
        m_level->m_textures[i].toTexture( glidos.get(), std::function<void(const std::string&)>(
                [this](const std::string& s) { drawLoadingScreen( s ); } ) );
    }

    drawLoadingScreen( "Preparing the game" );

    setUpRendering();

    if( useAlternativeLara )
    {
        useAlternativeLaraAppearance();
    }

    for( const auto& item : initInv )
        addInventoryItem( item.first, item.second );

    if( trackToPlay )
    {
        playStopCdTrack( trackToPlay.value(), false );
    }

    if( !cutsceneName.empty() )
    {
        m_cameraController
                ->setEyeRotation( 0_deg, core::Angle::fromDegrees( levelInfo.get<float>( "cameraRot" ) ) );
        auto pos = m_cameraController->getTRPosition().position;
        if( auto x = levelInfo["cameraPosX"] )
            pos.X = x;
        if( auto y = levelInfo["cameraPosY"] )
            pos.Y = y;
        if( auto z = levelInfo["cameraPosZ"] )
            pos.Z = z;

        m_cameraController->setPosition( pos );

        if( bool(levelInfo["flipRooms"]) )
            swapAllRooms();

        for( auto& room : m_level->m_rooms )
            room.node->setVisible( room.alternateRoom.get() < 0 );

        if( bool(levelInfo["gunSwap"]) )
        {
            const auto& laraPistol = findAnimatedModelForType( engine::TR1ItemId::LaraPistolsAnim );
            Expects( laraPistol != nullptr );
            for( const auto& item : m_itemNodes | boost::adaptors::map_values )
            {
                if( item->m_state.type != engine::TR1ItemId::CutsceneActor1 )
                    continue;

                item->getNode()->getChild( 1 )->setDrawable( laraPistol->models[1].get() );
                item->getNode()->getChild( 4 )->setDrawable( laraPistol->models[4].get() );
            }
        }
    }

    depthDarknessFx = std::make_shared<render::FullScreenFX>( *game,
                                                              gameplay::ShaderProgram::createFromFile(
                                                                      "shaders/fx_darkness.vert",
                                                                      "shaders/fx_darkness.frag",
                                                                      {"LENS_DISTORTION"} ) );
    depthDarknessFx->getMaterial()->getParameter( "aspect_ratio" )->bind(
            [this](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                uniform.set( game->getAspectRatio() );
            } );
    depthDarknessFx->getMaterial()->getParameter( "distortion_power" )->set( -1.0f );
    depthDarknessFx->getMaterial()->getParameter( "u_time" )->bind(
            [this](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>( game->getGameTime() );
                uniform.set( gsl::narrow_cast<float>( now.time_since_epoch().count() ) );
            }
    );

    depthDarknessWaterFx = std::make_shared<render::FullScreenFX>( *game,
                                                                   gameplay::ShaderProgram::createFromFile(
                                                                           "shaders/fx_darkness.vert",
                                                                           "shaders/fx_darkness.frag",
                                                                           {"WATER",
                                                                            "LENS_DISTORTION"} ) );
    depthDarknessWaterFx->getMaterial()->getParameter( "aspect_ratio" )->bind(
            [this](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                uniform.set( game->getAspectRatio() );
            } );
    depthDarknessWaterFx->getMaterial()->getParameter( "distortion_power" )->set( -2.0f );
    depthDarknessWaterFx->getMaterial()->getParameter( "u_time" )->bind(
            [this](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>( game->getGameTime() );
                uniform.set( gsl::narrow_cast<float>( now.time_since_epoch().count() ) );
            }
    );
}

void Engine::run()
{
    static const auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::seconds( 1 ) )
                                      / core::FrameRate.get();

    const std::string levelName = levelInfo["name"];

    bool showDebugInfo = false;
    bool showDebugInfoToggled = false;

    auto font = std::make_shared<gameplay::gl::Font>( "DroidSansMono.ttf", 12 );
    font->setTarget( screenOverlay->getImage() );

    const auto& trFontGraphics = m_level->m_spriteSequences.at( engine::TR1ItemId::FontGraphics );
    auto trFont = render::CachedFont( *trFontGraphics );

    auto nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;

    const bool isCutscene = !levelInfo.get<std::string>( "cutscene" ).empty();

    while( !game->windowShouldClose() )
    {
        screenOverlay->getImage()->fill( {0, 0, 0, 0} );

        if( !levelName.empty() )
        {
            render::Label tmp{0, -50, levelName};
            tmp.alignX = render::Label::Alignment::Center;
            tmp.alignY = render::Label::Alignment::Bottom;
            tmp.outline = true;
            tmp.addBackground( 0, 0, 0, 0 );
            tmp.draw( trFont, *screenOverlay->getImage(), *m_level->m_palette );
        }

        m_soundEngine.update();
        m_inputHandler->update();

        if( m_inputHandler->getInputState().debug )
        {
            if( !showDebugInfoToggled )
            {
                showDebugInfoToggled = true;
                showDebugInfo = !showDebugInfo;
            }
        }
        else
        {
            showDebugInfoToggled = false;
        }

        {
            // frame rate throttling
            // TODO this assumes that the frame rate capacity (the processing power so to speak)
            // is faster than 30 FPS.
            std::this_thread::sleep_until( nextFrameTime );
            nextFrameTime += frameDuration;
        }

        update( bool( m_scriptEngine["cheats"]["godMode"] ) );

        if( game->updateWindowSize() )
        {
            game->getScene()->getActiveCamera()->setAspectRatio( game->getAspectRatio() );
            depthDarknessFx->init( *game );
            depthDarknessWaterFx->init( *game );
            screenOverlay->init( game->getViewport() );
            font->setTarget( screenOverlay->getImage() );
        }

        if( !isCutscene )
        {
            m_cameraController->update();
        }
        else
        {
            if( ++m_cameraController->m_cinematicFrame >= m_level->m_cinematicFrames.size() )
                break;

            m_cameraController
                    ->updateCinematic( m_level->m_cinematicFrames[m_cameraController->m_cinematicFrame], false );
        }
        doGlobalEffect();

        if( m_lara != nullptr )
            drawBars( game.get(), screenOverlay->getImage() );

        if( m_cameraController->getCurrentRoom()->isWaterRoom() )
            depthDarknessWaterFx->bind();
        else
            depthDarknessFx->bind();
        game->render();

        gameplay::RenderContext context{};
        gameplay::Node dummyNode{""};
        context.setCurrentNode( &dummyNode );

        gameplay::gl::FrameBuffer::unbindAll();

        if( m_cameraController->getCurrentRoom()->isWaterRoom() )
            depthDarknessWaterFx->render( context );
        else
            depthDarknessFx->render( context );

        if( showDebugInfo )
        {
            drawDebugInfo( font, game->getFrameRate() );

            for( const std::shared_ptr<engine::items::ItemNode>& ctrl : m_itemNodes | boost::adaptors::map_values )
            {
                const auto vertex = glm::vec3{game->getScene()->getActiveCamera()->getViewMatrix()
                                              * glm::vec4( ctrl->getNode()->getTranslationWorld(), 1 )};

                if( vertex.z > -game->getScene()->getActiveCamera()->getNearPlane() )
                {
                    continue;
                }
                else if( vertex.z < -game->getScene()->getActiveCamera()->getFarPlane() )
                {
                    continue;
                }

                glm::vec4 projVertex{vertex, 1};
                projVertex = game->getScene()->getActiveCamera()->getProjectionMatrix() * projVertex;
                projVertex /= projVertex.w;

                if( std::abs( projVertex.x ) > 1 || std::abs( projVertex.y ) > 1 )
                    continue;

                projVertex.x = (projVertex.x / 2 + 0.5f) * game->getViewport().width;
                projVertex.y = (1 - (projVertex.y / 2 + 0.5f)) * game->getViewport().height;

                font->drawText( ctrl->getNode()->getId().c_str(), projVertex.x, projVertex.y,
                                gameplay::gl::RGBA8{255} );
            }
        }

        screenOverlay->draw( context );

        game->swapBuffers();

        if( m_inputHandler->getInputState().save )
        {
            scaleSplashImage();
            abibasFont->setTarget( screenOverlay->getImage() );
            drawLoadingScreen( "Saving..." );

            BOOST_LOG_TRIVIAL( info ) << "Save";
            std::ofstream file{"quicksave.yaml", std::ios::out | std::ios::trunc};
            Expects( file.is_open() );
            file << save();

            nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
        }
        else if( m_inputHandler->getInputState().load )
        {
            scaleSplashImage();
            abibasFont->setTarget( screenOverlay->getImage() );
            drawLoadingScreen( "Loading..." );

            BOOST_LOG_TRIVIAL( info ) << "Load";
            load( YAML::LoadFile( "quicksave.yaml" ) );

            nextFrameTime = std::chrono::high_resolution_clock::now() + frameDuration;
        }
    }
}

void Engine::scaleSplashImage()
{
    // scale splash image so that its aspect ratio is preserved, but the boundaries match
    const float splashScale = std::max(
            gsl::narrow<float>( game->getViewport().width ) / splashImage.width(),
            gsl::narrow<float>( game->getViewport().height ) / splashImage.height()
    );

    splashImageScaled = splashImage;
    splashImageScaled.resize( splashImageScaled.width() * splashScale, splashImageScaled.height() * splashScale );

    // crop to boundaries
    const auto centerX = splashImageScaled.width() / 2;
    const auto centerY = splashImageScaled.height() / 2;
    splashImageScaled.crop(
            gsl::narrow<int>( centerX - game->getViewport().width / 2 ),
            gsl::narrow<int>( centerY - game->getViewport().height / 2 ),
            gsl::narrow<int>( centerX - game->getViewport().width / 2 + game->getViewport().width - 1 ),
            gsl::narrow<int>( centerY - game->getViewport().height / 2 + game->getViewport().height - 1 )
    );

    Expects( splashImageScaled.width() == game->getViewport().width );
    Expects( splashImageScaled.height() == game->getViewport().height );

    splashImageScaled.interleave();
}

void Engine::drawLoadingScreen(const std::string& state)
{
    glfwPollEvents();
    if( game->updateWindowSize() )
    {
        game->getScene()->getActiveCamera()->setAspectRatio( game->getAspectRatio() );
        screenOverlay->init( game->getViewport() );
        abibasFont->setTarget( screenOverlay->getImage() );

        scaleSplashImage();
    }
    screenOverlay->getImage()->assign(
            reinterpret_cast<const gameplay::gl::RGBA8*>(splashImageScaled.data()),
            game->getViewport().width * game->getViewport().height
    );
    abibasFont->drawText( state, 40, gsl::narrow<int>( game->getViewport().height - 100 ), 255, 255, 255, 192 );

    gameplay::gl::FrameBuffer::unbindAll();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    gameplay::gl::checkGlError();

    game->clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, {0, 0, 0, 0}, 1 );
    gameplay::RenderContext context{};
    gameplay::Node dummyNode{""};
    context.setCurrentNode( &dummyNode );
    screenOverlay->draw( context );
    game->swapBuffers();
}

const std::vector<int16_t>& Engine::getPoseFrames() const
{
    return m_level->m_poseFrames;
}

const std::vector<loader::file::Animation>& Engine::getAnimations() const
{
    return m_level->m_animations;
}

const std::vector<uint16_t>& Engine::getOverlaps() const
{
    return m_level->m_overlaps;
}

const std::unique_ptr<loader::file::SkeletalModelType>& Engine::findAnimatedModelForType(TR1ItemId type) const
{
    return m_level->findAnimatedModelForType(type);
}

Engine::~Engine() = default;
}
