#include "level.h"

#include "engine/laranode.h"
#include "render/textureanimator.h"
#include "tr1level.h"
#include "tr2level.h"
#include "tr3level.h"
#include "tr4level.h"
#include "tr5level.h"

#include "engine/items/animating.h"
#include "engine/items/bat.h"
#include "engine/items/bear.h"
#include "engine/items/block.h"
#include "engine/items/boulder.h"
#include "engine/items/bridgeflat.h"
#include "engine/items/collapsiblefloor.h"
#include "engine/items/cutsceneactors.h"
#include "engine/items/dart.h"
#include "engine/items/dartgun.h"
#include "engine/items/door.h"
#include "engine/items/keyhole.h"
#include "engine/items/pickupitem.h"
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
#include "engine/items/trapdoordown.h"
#include "engine/items/trapdoorup.h"
#include "engine/items/underwaterswitch.h"
#include "engine/items/wolf.h"

#include "util/md5.h"

#include <boost/range/adaptors.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <glm/gtx/norm.hpp>

using namespace level;

Level::~Level() = default;

/// \brief reads the mesh data.
void Level::readMeshData(loader::io::SDLReader& reader)
{
    const auto meshDataWords = reader.readU32();
    const auto basePos = reader.tell();

    const auto meshDataSize = meshDataWords * 2;
    reader.skip( meshDataSize );

    reader.readVector( m_meshIndices, reader.readU32() );
    const auto endPos = reader.tell();

    m_meshes.clear();

    uint32_t meshDataPos = 0;
    for( uint32_t i = 0; i < m_meshIndices.size(); i++ )
    {
        replace( m_meshIndices.begin(), m_meshIndices.end(), meshDataPos, i );

        reader.seek( basePos + std::streamoff( meshDataPos ) );

        if( gameToEngine( m_gameVersion ) >= Engine::TR4 )
            m_meshes.emplace_back( *loader::Mesh::readTr4( reader ) );
        else
            m_meshes.emplace_back( *loader::Mesh::readTr1( reader ) );

        for( auto pos : m_meshIndices )
        {
            if( pos > meshDataPos )
            {
                meshDataPos = pos;
                break;
            }
        }
    }

    reader.seek( endPos );
}

std::shared_ptr<Level> Level::createLoader(const std::string& filename, Game gameVersion, sol::state&& scriptEngine)
{
    std::string sfxPath = (boost::filesystem::path( filename ).remove_filename() / "MAIN.SFX").string();

    loader::io::SDLReader reader( filename );
    if( !reader.isOpen() )
        return nullptr;

    if( gameVersion == Game::Unknown )
        gameVersion = probeVersion( reader, filename );
    if( gameVersion == Game::Unknown )
        return nullptr;

    reader.seek( 0 );
    return createLoader( std::move( reader ), gameVersion, sfxPath, std::move( scriptEngine ) );
}

std::shared_ptr<Level>
Level::createLoader(loader::io::SDLReader&& reader, Game game_version, const std::string& sfxPath,
                    sol::state&& scriptEngine)
{
    if( !reader.isOpen() )
        return nullptr;

    std::shared_ptr<Level> result;

    switch( game_version )
    {
        case Game::TR1:
            result = std::make_shared<level::TR1Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            break;
        case Game::TR1Demo:
        case Game::TR1UnfinishedBusiness:
            result = std::make_shared<level::TR1Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            result->m_demoOrUb = true;
            break;
        case Game::TR2:
            result = std::make_shared<level::TR2Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            break;
        case Game::TR2Demo:
            result = std::make_shared<level::TR2Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            result->m_demoOrUb = true;
            break;
        case Game::TR3:
            result = std::make_shared<level::TR3Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            break;
        case Game::TR4:
        case Game::TR4Demo:
            result = std::make_shared<level::TR4Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            break;
        case Game::TR5:
            result = std::make_shared<level::TR5Level>( game_version, std::move( reader ), std::move( scriptEngine ) );
            break;
        default:
            BOOST_THROW_EXCEPTION( std::runtime_error( "Invalid game version" ) );
    }

    result->m_sfxPath = sfxPath;
    return result;
}

Game Level::probeVersion(loader::io::SDLReader& reader, const std::string& filename)
{
    if( !reader.isOpen() || filename.length() < 5 )
        return Game::Unknown;

    std::string ext;
    ext += filename[filename.length() - 4];
    ext += (char)toupper( filename[filename.length() - 3] );
    ext += (char)toupper( filename[filename.length() - 2] );
    ext += (char)toupper( filename[filename.length() - 1] );

    reader.seek( 0 );
    uint8_t check[4];
    reader.readBytes( check, 4 );

    Game ret = Game::Unknown;
    if( ext == ".PHD" )
    {
        if( check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00 )
        {
            ret = Game::TR1;
        }
    }
    else if( ext == ".TUB" )
    {
        if( check[0] == 0x20 && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00 )
        {
            ret = Game::TR1UnfinishedBusiness;
        }
    }
    else if( ext == ".TR2" )
    {
        if( check[0] == 0x2D && check[1] == 0x00 && check[2] == 0x00 && check[3] == 0x00 )
        {
            ret = Game::TR2;
        }
        else if( (check[0] == 0x38 || check[0] == 0x34) && check[1] == 0x00 && (check[2] == 0x18 || check[2] == 0x08)
                 && check[3] == 0xFF )
        {
            ret = Game::TR3;
        }
    }
    else if( ext == ".TR4" )
    {
        if( check[0] == 0x54 && // T
            check[1] == 0x52 && // R
            check[2] == 0x34 && // 4
            check[3] == 0x00 )
        {
            ret = Game::TR4;
        }
        else if( check[0] == 0x54 && // T
                 check[1] == 0x52 && // R
                 check[2] == 0x34 && // 4
                 check[3] == 0x63 ) //
        {
            ret = Game::TR4;
        }
        else if( check[0] == 0xF0 && // T
                 check[1] == 0xFF && // R
                 check[2] == 0xFF && // 4
                 check[3] == 0xFF )
        {
            ret = Game::TR4;
        }
    }
    else if( ext == ".TRC" )
    {
        if( check[0] == 0x54 && // T
            check[1] == 0x52 && // R
            check[2] == 0x34 && // C
            check[3] == 0x00 )
        {
            ret = Game::TR5;
        }
    }

    return ret;
}

const loader::StaticMesh* Level::findStaticMeshById(uint32_t meshId) const
{
    for( const auto& mesh : m_staticMeshes )
        if( mesh.id == meshId )
            return &mesh;

    return nullptr;
}

int Level::findStaticMeshIndexById(uint32_t meshId) const
{
    for( const auto& mesh : m_staticMeshes )
    {
        if( mesh.id == meshId )
        {
            BOOST_ASSERT( mesh.mesh < m_meshIndices.size() );
            return m_meshIndices[mesh.mesh];
        }
    }

    return -1;
}

const std::unique_ptr<loader::SkeletalModelType>& Level::findAnimatedModelForType(engine::TR1ItemId type) const
{
    const auto it = m_animatedModels.find( type );
    if( it != m_animatedModels.end() )
        return it->second;

    static const std::unique_ptr<loader::SkeletalModelType> none;
    return none;
}

const std::unique_ptr<loader::SpriteSequence>& Level::findSpriteSequenceForType(engine::TR1ItemId type) const
{
    const auto it = m_spriteSequences.find( type );
    if( it != m_spriteSequences.end() )
        return it->second;

    static const std::unique_ptr<loader::SpriteSequence> none;
    return none;
}

std::map<loader::TextureLayoutProxy::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>
Level::createMaterials(const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader)
{
    const auto texMask = gameToEngine( m_gameVersion ) == Engine::TR4 ? loader::TextureIndexMaskTr4
                                                                      : loader::TextureIndexMask;
    std::map<loader::TextureLayoutProxy::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>> materials;
    for( loader::TextureLayoutProxy& proxy : m_textureProxies )
    {
        const auto& key = proxy.textureKey;
        if( materials.find( key ) != materials.end() )
            continue;

        materials.emplace( std::make_pair( key, proxy.createMaterial(
                gsl::make_not_null( m_textures[key.tileAndFlag & texMask].texture ), shader ) ) );
    }
    return materials;
}

engine::LaraNode* Level::createItems()
{
    engine::LaraNode* lara = nullptr;
    int id = -1;
    for( loader::Item& item : m_items )
    {
        ++id;

        BOOST_ASSERT( item.room < m_rooms.size() );
        auto room = gsl::make_not_null( const_cast<const loader::Room*>(&m_rooms[item.room]) );

        if( const auto& model = findAnimatedModelForType( item.type ) )
        {
            std::shared_ptr<engine::items::ItemNode> modelNode;

            if( item.type == engine::TR1ItemId::Lara )
            {
                modelNode = createSkeletalModel<engine::LaraNode>( *model, room, item );
                lara = static_cast<engine::LaraNode*>(modelNode.get());
            }
            else if( auto objectInfo = m_scriptEngine["getObjectInfo"].call( -1 ) )
            {
                BOOST_LOG_TRIVIAL( info ) << "Instantiating scripted type " << engine::toString( item.type ) << "/id "
                                          << id;

                modelNode = std::make_shared<engine::items::ScriptedItem>( gsl::make_not_null( this ),
                                                                           std::string( "skeleton(type:" )
                                                                           + engine::toString( item.type ) + ")",
                                                                           room,
                                                                           item,
                                                                           *model,
                                                                           objectInfo );
                for( gsl::index boneIndex = 0; boneIndex < model->models.size(); ++boneIndex )
                {
                    auto node = make_not_null_shared<gameplay::Node>(
                            modelNode->getNode()->getId() + "/bone:" + std::to_string( boneIndex ) );
                    node->setDrawable( model->models[boneIndex].get() );
                    addChild( gsl::make_not_null( modelNode->getNode() ), node );
                }

                BOOST_ASSERT( modelNode->getNode()->getChildren().size() == model->meshes.size() );
            }
            else if( item.type == engine::TR1ItemId::Wolf )
            {
                modelNode = createSkeletalModel<engine::items::Wolf>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::Bear )
            {
                modelNode = createSkeletalModel<engine::items::Bear>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::Bat )
            {
                modelNode = createSkeletalModel<engine::items::Bat>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::FallingBlock )
            {
                modelNode = createSkeletalModel<engine::items::CollapsibleFloor>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::SwingingBlade )
            {
                modelNode = createSkeletalModel<engine::items::SwingingBlade>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::RollingBall )
            {
                modelNode = createSkeletalModel<engine::items::RollingBall>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::Dart )
            {
                modelNode = createSkeletalModel<engine::items::Dart>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::DartEmitter )
            {
                modelNode = createSkeletalModel<engine::items::DartGun>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::LiftingDoor )
            {
                modelNode = createSkeletalModel<engine::items::TrapDoorUp>( *model, room, item );
            }
            else if( item.type >= engine::TR1ItemId::PushableBlock1 && item.type <= engine::TR1ItemId::PushableBlock4 )
            {
                modelNode = createSkeletalModel<engine::items::Block>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::MovingBlock )
            {
                modelNode = createSkeletalModel<engine::items::TallBlock>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::WallSwitch )
            {
                modelNode = createSkeletalModel<engine::items::Switch>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::UnderwaterSwitch )
            {
                modelNode = createSkeletalModel<engine::items::UnderwaterSwitch>( *model, room, item );
            }
            else if( item.type >= engine::TR1ItemId::Door1 && item.type <= engine::TR1ItemId::Door8 )
            {
                modelNode = createSkeletalModel<engine::items::Door>( *model, room, item );
            }
            else if( item.type >= engine::TR1ItemId::Trapdoor1 && item.type <= engine::TR1ItemId::Trapdoor2 )
            {
                modelNode = createSkeletalModel<engine::items::TrapDoorDown>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::BridgeFlat )
            {
                modelNode = createSkeletalModel<engine::items::BridgeFlat>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::BridgeTilt1 )
            {
                modelNode = createSkeletalModel<engine::items::BridgeSlope1>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::BridgeTilt2 )
            {
                modelNode = createSkeletalModel<engine::items::BridgeSlope2>( *model, room, item );
            }
            else if( item.type >= engine::TR1ItemId::Keyhole1 && item.type <= engine::TR1ItemId::Keyhole4 )
            {
                modelNode = createSkeletalModel<engine::items::KeyHole>( *model, room, item );
            }
            else if( item.type >= engine::TR1ItemId::PuzzleHole1 && item.type <= engine::TR1ItemId::PuzzleHole4 )
            {
                modelNode = createSkeletalModel<engine::items::PuzzleHole>( *model, room, item );
            }
            else if( item.type >= engine::TR1ItemId::Animating1 && item.type <= engine::TR1ItemId::Animating3 )
            {
                modelNode = createSkeletalModel<engine::items::Animating>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::TeethSpikes )
            {
                modelNode = createSkeletalModel<engine::items::TeethSpikes>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::Raptor )
            {
                modelNode = createSkeletalModel<engine::items::Raptor>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::SwordOfDamocles || item.type == engine::TR1ItemId::FallingCeiling )
            {
                modelNode = createSkeletalModel<engine::items::SwordOfDamocles>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::CutsceneActor1 )
            {
                modelNode = createSkeletalModel<engine::items::CutsceneActor1>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::CutsceneActor2 )
            {
                modelNode = createSkeletalModel<engine::items::CutsceneActor2>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::CutsceneActor3 )
            {
                modelNode = createSkeletalModel<engine::items::CutsceneActor3>( *model, room, item );
            }
            else if( item.type == engine::TR1ItemId::CutsceneActor4 )
            {
                modelNode = createSkeletalModel<engine::items::CutsceneActor4>( *model, room, item );
            }
            else
            {
                modelNode = createSkeletalModel<engine::items::StubItem>( *model, room, item );
                if( item.type == engine::TR1ItemId::MidasGoldTouch
                    || item.type == engine::TR1ItemId::CameraTarget
                    || item.type == engine::TR1ItemId::WaterfallMist
                    || item.type == engine::TR1ItemId::LavaParticleEmitter
                    || item.type == engine::TR1ItemId::FlameEmitter
                    || item.type == engine::TR1ItemId::Earthquake )
                {
                    modelNode->getNode()->setDrawable( nullptr );
                    modelNode->getNode()->removeAllChildren();
                }
                else
                {
                    BOOST_LOG_TRIVIAL( warning ) << "Unimplemented item " << toString( item.type );
                }
            }

            m_itemNodes[id] = modelNode;
            addChild( gsl::make_not_null( room->node ), gsl::make_not_null( modelNode->getNode() ) );

            modelNode->applyTransform();
            modelNode->updateLighting();

            continue;
        }

        if( const auto& spriteSequence = findSpriteSequenceForType( item.type ) )
        {
            BOOST_ASSERT( !findAnimatedModelForType( item.type ) );
            BOOST_ASSERT( spriteSequence->offset < m_sprites.size() );

            const loader::Sprite& sprite = m_sprites[spriteSequence->offset];
            std::shared_ptr<engine::items::ItemNode> node;

            if( item.type == engine::TR1ItemId::ScionPiece )
            {
                node = std::make_shared<engine::items::ScionPieceItem>( gsl::make_not_null( this ),
                                                                        std::string( "sprite(type:" )
                                                                        + engine::toString( item.type ) + ")",
                                                                        room,
                                                                        item,
                                                                        sprite,
                                                                        gsl::make_not_null( m_spriteMaterial ) );
            }
            else if( item.type == engine::TR1ItemId::Item141
                     || item.type == engine::TR1ItemId::Item142
                     || item.type == engine::TR1ItemId::Key1Sprite
                     || item.type == engine::TR1ItemId::Key2Sprite
                     || item.type == engine::TR1ItemId::Key3Sprite
                     || item.type == engine::TR1ItemId::Key4Sprite
                     || item.type == engine::TR1ItemId::Puzzle1Sprite
                     || item.type == engine::TR1ItemId::Puzzle2Sprite
                     || item.type == engine::TR1ItemId::Puzzle3Sprite
                     || item.type == engine::TR1ItemId::Puzzle4Sprite
                     || item.type == engine::TR1ItemId::PistolsSprite
                     || item.type == engine::TR1ItemId::ShotgunSprite
                     || item.type == engine::TR1ItemId::MagnumsSprite
                     || item.type == engine::TR1ItemId::UzisSprite
                     || item.type == engine::TR1ItemId::PistolAmmoSprite
                     || item.type == engine::TR1ItemId::ShotgunAmmoSprite
                     || item.type == engine::TR1ItemId::MagnumAmmoSprite
                     || item.type == engine::TR1ItemId::UziAmmoSprite
                     || item.type == engine::TR1ItemId::ExplosiveSprite
                     || item.type == engine::TR1ItemId::SmallMedipackSprite
                     || item.type == engine::TR1ItemId::LargeMedipackSprite
                     || item.type == engine::TR1ItemId::Item144
                     || item.type == engine::TR1ItemId::LeadBarSprite )
            {
                node = std::make_shared<engine::items::PickupItem>( gsl::make_not_null( this ),
                                                                    std::string( "sprite(type:" )
                                                                    + engine::toString( item.type ) + ")",
                                                                    room,
                                                                    item,
                                                                    sprite,
                                                                    gsl::make_not_null( m_spriteMaterial ) );
            }
            else
            {
                BOOST_LOG_TRIVIAL( warning ) << "Unimplemented item " << toString( item.type );
                node = std::make_shared<engine::items::SpriteItemNode>( gsl::make_not_null( this ),
                                                                        std::string( "sprite(type:" )
                                                                        + engine::toString( item.type ) + ")",
                                                                        room,
                                                                        item,
                                                                        true,
                                                                        sprite,
                                                                        gsl::make_not_null( m_spriteMaterial ) );
            }

            m_itemNodes[id] = node;
            addChild( gsl::make_not_null( room->node ), gsl::make_not_null( node->getNode() ) );

            node->applyTransform();

            continue;
        }

        BOOST_LOG_TRIVIAL( error ) << "Failed to find an appropriate animated model for item " << id << "/type "
                                   << int( item.type );
    }

    return lara;
}

template<typename T>
std::shared_ptr<T> Level::createSkeletalModel(const loader::SkeletalModelType& model,
                                              const gsl::not_null<const loader::Room*>& room,
                                              const loader::Item& item)
{
    static_assert( std::is_base_of<engine::items::ItemNode, T>::value, "T must be derived from ItemNode" );

    if( model.animation == nullptr )
    {
        BOOST_LOG_TRIVIAL( error ) << "Model 0x" << std::hex << reinterpret_cast<uintptr_t>(&model) << std::dec
                                   << " has no associated animation";
        return nullptr;
    }

    auto skeletalModel = std::make_shared<T>( gsl::make_not_null( this ),
                                              std::string( "skeleton(type:" ) + engine::toString( item.type ) + ")",
                                              room,
                                              item,
                                              model );
    for( gsl::index boneIndex = 0; boneIndex < model.meshes.size(); ++boneIndex )
    {
        auto node = make_not_null_shared<gameplay::Node>(
                skeletalModel->getNode()->getId() + "/bone:" + std::to_string( boneIndex ) );
        node->setDrawable( model.models[boneIndex].get() );
        addChild( gsl::make_not_null( skeletalModel->getNode() ), node );
    }

    BOOST_ASSERT( skeletalModel->getNode()->getChildren().size() == gsl::narrow<size_t>( model.meshes.size() ) );

    skeletalModel->getSkeleton()->updatePose( skeletalModel->m_state );

    return skeletalModel;
}

void Level::setUpRendering(const gsl::not_null<gameplay::Game*>& game)
{
    m_inputHandler = std::make_unique<engine::InputHandler>( gsl::make_not_null( game->getWindow() ) );

    for( auto& sprite : m_sprites )
    {
        BOOST_ASSERT( sprite.texture_id < m_textures.size() );
        sprite.texture = m_textures[sprite.texture_id].texture;
        sprite.image = m_textures[sprite.texture_id].image;
    }

    auto texturedShader = gsl::make_not_null( gameplay::ShaderProgram::createFromFile( "shaders/textured_2.vert",
                                                                                  "shaders/textured_2.frag" ) );
    auto materials = createMaterials( texturedShader );

    auto colorMaterial = make_not_null_shared<gameplay::Material>( "shaders/colored_2.vert",
                                                                   "shaders/colored_2.frag" );
    colorMaterial->getParameter( "u_modelMatrix" )->bindModelMatrix();
    colorMaterial->getParameter( "u_modelViewMatrix" )->bindModelViewMatrix();
    colorMaterial->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    BOOST_ASSERT( m_spriteMaterial == nullptr );
    m_spriteMaterial = make_not_null_shared<gameplay::Material>( "shaders/textured_2.vert", "shaders/textured_2.frag" );
    m_spriteMaterial->getRenderState().setCullFace( false );

    m_spriteMaterial->getParameter( "u_modelMatrix" )->bindModelMatrix();
    m_spriteMaterial->getParameter( "u_projectionMatrix" )->bindProjectionMatrix();

    m_spriteMaterial->getParameter( "u_baseLightDiff" )->set( 0.0f );
    m_spriteMaterial->getParameter( "u_lightPosition" )->set( glm::vec3{std::numeric_limits<float>::quiet_NaN()} );

    m_textureAnimator = std::make_shared<render::TextureAnimator>( m_animatedTextures );

    for( auto& mesh : m_meshes )
    {
        m_models.emplace_back( mesh.createModel( m_textureProxies, materials, colorMaterial, *m_palette,
                                                 *m_textureAnimator ) );
    }

    for( auto idx : m_meshIndices )
    {
        Expects( idx < m_models.size() );
        m_modelsDirect.emplace_back( m_models[idx] );
        m_meshesDirect.emplace_back( &m_meshes[idx] );
    }

    for( const std::unique_ptr<loader::SkeletalModelType>& model : m_animatedModels | boost::adaptors::map_values )
    {
        Expects( model->mesh_base_index + model->nmeshes <= m_modelsDirect.size() );
        if( model->nmeshes > 0 )
        {
            model->models = gsl::make_span( &m_modelsDirect[model->mesh_base_index], model->nmeshes );
            model->meshes = gsl::make_span( &m_meshesDirect[model->mesh_base_index], model->nmeshes );
        }
    }

    game->getScene()->setActiveCamera(
            std::make_shared<gameplay::Camera>( glm::radians( 80.0f ), game->getAspectRatio(), 10.0f, 20480.0f ) );

    auto waterTexturedShader = gsl::make_not_null( gameplay::ShaderProgram::createFromFile( "shaders/textured_2.vert",
                                                                                       "shaders/textured_2.frag",
                                                                                       {"WATER"} ) );
    auto waterMaterials = createMaterials( waterTexturedShader );
    for( const auto& m : waterMaterials | boost::adaptors::map_values )
    {
        m->getParameter( "u_time" )->bind(
                [&game](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                    const auto now = std::chrono::time_point_cast<std::chrono::milliseconds>( game->getGameTime() );
                    uniform.set( gsl::narrow_cast<float>( now.time_since_epoch().count() ) );
                }
        );
    }

    for( size_t i = 0; i < m_rooms.size(); ++i )
    {
        m_rooms[i].createSceneNode( i, *this, materials, waterMaterials, m_models, *m_textureAnimator );
        game->getScene()->addNode( gsl::make_not_null( m_rooms[i].node ) );
    }

    m_lara = createItems();
    if( m_lara == nullptr )
    {
        m_cameraController = std::make_unique<engine::CameraController>(
                gsl::make_not_null( this ),
                gsl::make_not_null( game->getScene()->getActiveCamera() ),
                true );

        for( const auto& item : m_items )
        {
            if( item.type == engine::TR1ItemId::CutsceneActor1 )
            {
                m_cameraController->setPosition( item.position );
            }
        }
    }
    else
    {
        m_cameraController = std::make_unique<engine::CameraController>(
                gsl::make_not_null( this ),
                gsl::make_not_null( game->getScene()->getActiveCamera() ) );
    }

    for( const loader::SoundSource& src : m_soundSources )
    {
        auto handle = gsl::make_not_null( playSound( src.sound_id, src.position.toRenderSystem() ) );
        handle->setLooping( true );
    }
}

void Level::convertTexture(loader::ByteTexture& tex, loader::Palette& pal, loader::DWordTexture& dst)
{
    for( int y = 0; y < 256; y++ )
    {
        for( int x = 0; x < 256; x++ )
        {
            int col = tex.pixels[y][x];

            if( col > 0 )
                dst.pixels[y][x] = {pal.colors[col].r, pal.colors[col].g, pal.colors[col].b, 255};
            else
                dst.pixels[y][x] = {0, 0, 0, 0};
        }
    }

    dst.md5 = util::md5( &tex.pixels[0][0], 256 * 256 );
}

void Level::convertTexture(loader::WordTexture& tex, loader::DWordTexture& dst)
{
    for( int y = 0; y < 256; y++ )
    {
        for( int x = 0; x < 256; x++ )
        {
            int col = tex.pixels[y][x];

            if( (col & 0x8000) != 0 )
            {
                const uint8_t r = static_cast<const uint8_t>((col & 0x00007c00) >> 7);
                const uint8_t g = static_cast<const uint8_t>((col & 0x000003e0) >> 2);
                const uint8_t b = static_cast<const uint8_t>((col & 0x0000001f) << 3);
                dst.pixels[y][x] = {r, g, b, 1};
            }
            else
            {
                dst.pixels[y][x] = {0, 0, 0, 0};
            }
        }
    }
}

const loader::Sector* Level::findRealFloorSector(const core::TRVec& position,
                                                 const gsl::not_null<gsl::not_null<const loader::Room*>*>& room) const
{
    const loader::Sector* sector = nullptr;
    while( true )
    {
        sector = (*room)->findFloorSectorWithClampedIndex( (position.X - (*room)->position.X) / loader::SectorSize,
                                                           (position.Z - (*room)->position.Z) / loader::SectorSize );
        if( sector->portalTarget == nullptr )
        {
            break;
        }

        *room = gsl::make_not_null( sector->portalTarget );
    }

    Expects( sector != nullptr );
    if( sector->floorHeight * loader::QuarterSectorSize > position.Y )
    {
        while( sector->ceilingHeight * loader::QuarterSectorSize > position.Y && sector->roomAbove != nullptr )
        {
            *room = gsl::make_not_null( sector->roomAbove );
            sector = (*room)->getSectorByAbsolutePosition( position );
            if( sector == nullptr )
                return nullptr;
        }
    }
    else
    {
        while( sector->floorHeight * loader::QuarterSectorSize < position.Y && sector->roomBelow != nullptr )
        {
            *room = gsl::make_not_null( sector->roomBelow );
            sector = (*room)->getSectorByAbsolutePosition( position );
            if( sector == nullptr )
                return nullptr;
        }
    }

    return sector;
}

gsl::not_null<const loader::Room*>
Level::findRoomForPosition(const core::TRVec& position, gsl::not_null<const loader::Room*> room) const
{
    const loader::Sector* sector = nullptr;
    while( true )
    {
        sector = room->findFloorSectorWithClampedIndex(
                gsl::narrow_cast<int>( position.X - room->position.X ) / loader::SectorSize,
                gsl::narrow_cast<int>( position.Z - room->position.Z ) / loader::SectorSize );
        Expects( sector != nullptr );
        if( sector->portalTarget == nullptr )
        {
            break;
        }

        room = gsl::make_not_null( sector->portalTarget );
    }

    Expects( sector != nullptr );
    if( sector->floorHeight * loader::QuarterSectorSize > position.Y )
    {
        while( sector->ceilingHeight * loader::QuarterSectorSize > position.Y && sector->roomAbove != nullptr )
        {
            room = gsl::make_not_null( sector->roomAbove );
            sector = room->getSectorByAbsolutePosition( position );
            Expects( sector != nullptr );
        }
    }
    else
    {
        while( sector->floorHeight * loader::QuarterSectorSize <= position.Y && sector->roomBelow != nullptr )
        {
            room = gsl::make_not_null( sector->roomBelow );
            sector = room->getSectorByAbsolutePosition( position );
            Expects( sector != nullptr );
        }
    }

    return room;
}

engine::items::ItemNode* Level::getItemController(uint16_t id) const
{
    auto it = m_itemNodes.find( id );
    if( it == m_itemNodes.end() )
        return nullptr;

    return it->second.get();
}

void Level::drawBars(const gsl::not_null<gameplay::Game*>& game,
                     const gsl::not_null<std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>>>& image) const
{
    if( m_lara->isInWater() )
    {
        const GLint x0 = gsl::narrow<GLint>( game->getViewport().width - 110 );

        for( int i = 7; i <= 13; ++i )
            image->line( x0 - 1, i, x0 + 101, i, m_palette->colors[0].toTextureColor() );
        image->line( x0 - 2, 14, x0 + 102, 14, m_palette->colors[17].toTextureColor() );
        image->line( x0 + 102, 6, x0 + 102, 14, m_palette->colors[17].toTextureColor() );
        image->line( x0 + 102, 6, x0 + 102, 14, m_palette->colors[19].toTextureColor() );
        image->line( x0 - 2, 6, x0 - 2, 14, m_palette->colors[19].toTextureColor() );

        const int p = util::clamp( m_lara->getAir() * 100 / core::LaraAir, 0, 100 );
        if( p > 0 )
        {
            image->line( x0, 8, x0 + p, 8, m_palette->colors[32].toTextureColor() );
            image->line( x0, 9, x0 + p, 9, m_palette->colors[41].toTextureColor() );
            image->line( x0, 10, x0 + p, 10, m_palette->colors[32].toTextureColor() );
            image->line( x0, 11, x0 + p, 11, m_palette->colors[19].toTextureColor() );
            image->line( x0, 12, x0 + p, 12, m_palette->colors[21].toTextureColor() );
        }
    }

    const int x0 = 8;
    for( int i = 7; i <= 13; ++i )
        image->line( x0 - 1, i, x0 + 101, i, m_palette->colors[0].toTextureColor() );
    image->line( x0 - 2, 14, x0 + 102, 14, m_palette->colors[17].toTextureColor() );
    image->line( x0 + 102, 6, x0 + 102, 14, m_palette->colors[17].toTextureColor() );
    image->line( x0 + 102, 6, x0 + 102, 14, m_palette->colors[19].toTextureColor() );
    image->line( x0 - 2, 6, x0 - 2, 14, m_palette->colors[19].toTextureColor() );

    const int p = util::clamp( m_lara->m_state.health * 100 / core::LaraHealth, 0, 100 );
    if( p > 0 )
    {
        image->line( x0, 8, x0 + p, 8, m_palette->colors[8].toTextureColor() );
        image->line( x0, 9, x0 + p, 9, m_palette->colors[11].toTextureColor() );
        image->line( x0, 10, x0 + p, 10, m_palette->colors[8].toTextureColor() );
        image->line( x0, 11, x0 + p, 11, m_palette->colors[6].toTextureColor() );
        image->line( x0, 12, x0 + p, 12, m_palette->colors[24].toTextureColor() );
    }
}

void Level::triggerCdTrack(uint16_t trackId, const engine::floordata::ActivationState& activationRequest,
                           engine::floordata::SequenceCondition triggerType)
{
    if( trackId < 1 || trackId >= 64 )
        return;

    if( trackId < 28 )
    {
        // 1..27
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == 28 )
    {
        // 28
        if( m_cdTrackActivationStates[trackId].isOneshot()
            && m_lara->getCurrentAnimState() == loader::LaraStateId::JumpUp )
        {
            trackId = 29;
        }
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId < 41 )
    {
        // 29..40
        if( trackId != 37 )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == 41 )
    {
        // 41
        if( m_lara->getCurrentAnimState() == loader::LaraStateId::Hang )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == 42 )
    {
        // 42
        if( m_lara->getCurrentAnimState() == loader::LaraStateId::Hang )
            triggerNormalCdTrack( 43, activationRequest, triggerType );
        else
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId < 49 )
    {
        // 43..48
        triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == 49 )
    {
        // 49
        if( m_lara->getCurrentAnimState() == loader::LaraStateId::OnWaterStop )
            triggerNormalCdTrack( trackId, activationRequest, triggerType );
    }
    else if( trackId == 50 )
    {
        // 50
        if( m_cdTrackActivationStates[trackId].isOneshot() )
        {
            if( ++m_cdTrack50time == 120 )
            {
                m_levelFinished = true;
                m_cdTrack50time = 0;
                triggerNormalCdTrack( trackId, activationRequest, triggerType );
            }
        }
        else if( m_lara->getCurrentAnimState() == loader::LaraStateId::OnWaterExit )
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

void Level::triggerNormalCdTrack(uint16_t trackId, const engine::floordata::ActivationState& activationRequest,
                                 engine::floordata::SequenceCondition triggerType)
{
    if( m_cdTrackActivationStates[trackId].isOneshot() )
        return;

    if( triggerType == engine::floordata::SequenceCondition::ItemActivated )
        m_cdTrackActivationStates[trackId] ^= activationRequest.getActivationSet();
    else if( triggerType == engine::floordata::SequenceCondition::LaraOnGroundInverted )
        m_cdTrackActivationStates[trackId] &= ~activationRequest.getActivationSet();
    else
        m_cdTrackActivationStates[trackId] |= activationRequest.getActivationSet();

    if( !m_cdTrackActivationStates[trackId].isFullyActivated() )
    {
        stopCdTrack( trackId );
        return;
    }

    if( activationRequest.isOneshot() )
        m_cdTrackActivationStates[trackId].setOneshot( true );

    if( m_activeCDTrack != trackId )
        playCdTrack( trackId );
}

void Level::playCdTrack(uint16_t trackId)
{
    if( trackId == 13 )
    {
        playSound( 173, boost::none );
        return;
    }

    if( trackId > 2 && trackId < 22 )
        return;

    BOOST_LOG_TRIVIAL( debug ) << "Stopping track #" << m_activeCDTrack;

    if( m_activeCDTrack >= 26 && m_activeCDTrack <= 56 )
    {
        stopSoundEffect( gsl::narrow<uint16_t>( m_activeCDTrack + 148 ) );
    }
    else if( m_activeCDTrack > 0 )
    {
        if( !m_cdStream.expired() )
            m_audioDev.removeStream( m_cdStream.lock() );
    }
    m_activeCDTrack = 0;

    BOOST_LOG_TRIVIAL( debug ) << "Playing track #" << trackId;

    if( trackId >= 26 && trackId <= 56 )
    {
        m_lara->playSoundEffect( trackId + 148 );
    }
    else if( trackId >= 22 && trackId <= 25 )
    {
        // 22..25 -> 7..10: cinematic music
        playStream( trackId - 15 );
    }
    else if( trackId == 2 )
    {
        // 2: title music
        playStream( trackId );
    }
    else if( trackId > 56 )
    {
        // 57..60 -> 3..6: ambient+cinematic music
        playStream( trackId - 54 );
    }

    m_activeCDTrack = trackId;
}

void Level::stopCdTrack(uint16_t trackId)
{
    if( m_activeCDTrack == 0 )
        return;

    if( m_activeCDTrack < 26 || m_activeCDTrack > 56 )
    {
        if( !m_cdStream.expired() )
            m_audioDev.removeStream( m_cdStream.lock() );
    }
    else
    {
        stopSoundEffect( static_cast<uint16_t>(trackId + 148) );
    }

    m_activeCDTrack = 0;
}

void Level::playStream(uint16_t trackId)
{
    static constexpr size_t DefaultBufferSize = 16384;

    if( !m_cdStream.expired() )
        m_audioDev.removeStream( m_cdStream.lock() );

    if( boost::filesystem::is_regular_file( "data/tr1/audio/CDAUDIO.WAD" ) )
        m_cdStream = m_audioDev.createStream(
                std::make_unique<audio::WadStreamSource>( "data/tr1/audio/CDAUDIO.WAD", trackId ),
                DefaultBufferSize ).get();
    else
        m_cdStream = m_audioDev.createStream(
                std::make_unique<audio::SndfileStreamSource>(
                        (boost::format( "data/tr1/audio/%03d.ogg" ) % trackId).str() ),
                DefaultBufferSize ).get();
}

void Level::useAlternativeLaraAppearance()
{
    const auto& base = *m_animatedModels[engine::TR1ItemId::Lara];
    BOOST_ASSERT( base.models.size() == m_lara->getNode()->getChildren().size() );

    const auto& alternate = *m_animatedModels[engine::TR1ItemId::AlternativeLara];
    BOOST_ASSERT( alternate.models.size() == m_lara->getNode()->getChildren().size() );

    for( size_t i = 0; i < m_lara->getNode()->getChildren().size(); ++i )
        m_lara->getNode()->getChild( i )->setDrawable( alternate.models[i].get() );

    // Don't replace the head.
    m_lara->getNode()->getChild( 14 )->setDrawable( base.models[14].get() );
}

void Level::postProcessDataStructures()
{
    BOOST_LOG_TRIVIAL( info ) << "Post-processing data structures";

    for( loader::Room& room : m_rooms )
    {
        for( loader::Sector& sector : room.sectors )
        {
            if( sector.boxIndex >= 0 )
            {
                Expects( static_cast<size_t>(sector.boxIndex) < m_boxes.size() );
                sector.box = &m_boxes[sector.boxIndex];
            }

            if( sector.roomIndexBelow != 0xff )
            {
                Expects( sector.roomIndexBelow < m_rooms.size() );
                sector.roomBelow = &m_rooms[sector.roomIndexBelow];
            }

            if( sector.roomIndexAbove != 0xff )
            {
                Expects( sector.roomIndexAbove < m_rooms.size() );
                sector.roomAbove = &m_rooms[sector.roomIndexAbove];
            }

            if( sector.floorDataIndex != 0 )
            {
                sector.floorData = &m_floorData[sector.floorDataIndex];

                const auto portalTarget = engine::floordata::getPortalTarget( sector.floorData );
                if( portalTarget.is_initialized() )
                {
                    BOOST_ASSERT( *portalTarget != 0xff && *portalTarget < m_rooms.size() );
                    sector.portalTarget = &m_rooms[*portalTarget];
                }
            }
        }
    }

    Expects( m_baseZones.flyZone.size() == m_boxes.size() );
    Expects( m_baseZones.groundZone1.size() == m_boxes.size() );
    Expects( m_baseZones.groundZone2.size() == m_boxes.size() );
    Expects( m_alternateZones.flyZone.size() == m_boxes.size() );
    Expects( m_alternateZones.groundZone1.size() == m_boxes.size() );
    Expects( m_alternateZones.groundZone2.size() == m_boxes.size() );
    for( size_t i = 0; i < m_boxes.size(); ++i )
    {
        m_boxes[i].zoneFly = m_baseZones.flyZone[i];
        m_boxes[i].zoneGround1 = m_baseZones.groundZone1[i];
        m_boxes[i].zoneGround2 = m_baseZones.groundZone2[i];
        m_boxes[i].zoneFlySwapped = m_alternateZones.flyZone[i];
        m_boxes[i].zoneGround1Swapped = m_alternateZones.groundZone1[i];
        m_boxes[i].zoneGround2Swapped = m_alternateZones.groundZone2[i];
    }

    for( const std::unique_ptr<loader::SkeletalModelType>& model : m_animatedModels | boost::adaptors::map_values )
    {
        Expects( model->pose_data_offset % 2 == 0 );

        const auto idx = model->pose_data_offset / 2;
        if( idx >= m_poseFrames.size() )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Pose frame data index " << idx << " out of range 0.."
                                         << m_poseFrames.size() - 1;
            continue;
        }
        model->frames = reinterpret_cast<const loader::AnimFrame*>(&m_poseFrames[idx]);
        if( model->nmeshes > 1 )
        {
            model->boneTree = gsl::make_span(
                    reinterpret_cast<const loader::BoneTreeEntry*>(&m_boneTrees[model->bone_index]),
                    model->nmeshes - 1 );
        }

        Expects( model->animation_index == 0xffff || model->animation_index < m_animations.size() );
        if( model->animation_index != 0xffff )
            model->animation = &m_animations[model->animation_index];
    }

    for( loader::Animation& anim : m_animations )
    {
        Expects( anim.poseDataOffset % 2 == 0 );

        const auto idx = anim.poseDataOffset / 2;
        if( idx >= m_poseFrames.size() )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Pose frame data index " << idx << " out of range 0.."
                                         << m_poseFrames.size() - 1;
        }
        else
        {
            anim.frames = reinterpret_cast<const loader::AnimFrame*>(&m_poseFrames[idx]);
        }

        Expects( anim.nextAnimationIndex < m_animations.size() );
        anim.nextAnimation = &m_animations[anim.nextAnimationIndex];

        Expects( anim.animCommandIndex + anim.animCommandCount <= m_animCommands.size() );

        Expects( anim.transitionsIndex + anim.transitionsCount <= m_transitions.size() );
        if( anim.transitionsCount > 0 )
            anim.transitions = gsl::make_span( &m_transitions[anim.transitionsIndex],
                                               anim.transitionsCount );
    }

    for( loader::TransitionCase& transitionCase : m_transitionCases )
    {
        if( transitionCase.targetAnimationIndex < m_animations.size() )
            transitionCase.targetAnimation = &m_animations[transitionCase.targetAnimationIndex];
        else
            BOOST_LOG_TRIVIAL( warning ) << "Animation index " << transitionCase.targetAnimationIndex
                                         << " not less than " << m_animations.size();
    }

    for( loader::Transitions& transition : m_transitions )
    {
        Expects( transition.firstTransitionCase + transition.transitionCaseCount <= m_transitionCases.size() );
        if( transition.transitionCaseCount > 0 )
            transition.transitionCases = gsl::make_span(
                    &m_transitionCases[transition.firstTransitionCase],
                    transition.transitionCaseCount );
    }
}

void Level::dinoStompEffect(engine::items::ItemNode& node)
{
    const auto d = node.m_state.position.position.toRenderSystem() - m_cameraController->getPosition();
    const auto absD = glm::abs( d );

    const auto MaxD = 16 * loader::SectorSize;
    if( absD.x > MaxD || absD.y > MaxD || absD.z > MaxD )
        return;

    auto x = 1 - glm::length2( d ) / util::square( MaxD );
    m_cameraController->setBounce( 100 * x );
}

void Level::turn180Effect(engine::items::ItemNode& node)
{
    node.m_state.rotation.Y += 180_deg;
}

void Level::laraNormalEffect()
{
    Expects( m_lara != nullptr );
    m_lara->setCurrentAnimState( engine::LaraStateId::Stop );
    m_lara->setRequiredAnimState( engine::LaraStateId::Unknown12 );
    m_lara->m_state.anim = &m_animations[static_cast<int>(loader::AnimationId::STAY_SOLID)];
    m_lara->m_state.frame_number = 185;
    m_cameraController->setMode( engine::CameraMode::Chase );
    m_cameraController->getCamera()->setFieldOfView( glm::radians( 80.0f ) );
}

void Level::laraBubblesEffect(engine::items::ItemNode& node)
{
    auto modelNode = dynamic_cast<engine::items::ModelItemNode*>(&node);
    if( modelNode == nullptr )
        return;

    auto bubbleCount = util::rand15( 3 );
    if( bubbleCount != 0 )
    {
        node.playSoundEffect( 37 );

        const auto itemSpheres = modelNode->getSkeleton()->getBoneCollisionSpheres(
                node.m_state,
                *modelNode->getSkeleton()->getInterpolationInfo( modelNode->m_state ).getNearestFrame(),
                nullptr );

        auto position = core::TRVec{
                glm::vec3{glm::translate( itemSpheres[14].m,
                                          core::TRVec{0, 0, 50}.toRenderSystem() )[3]}};

        while( bubbleCount-- > 0 )
        {
            auto particle = make_not_null_shared<engine::BubbleParticle>(
                    core::RoomBoundPosition{node.m_state.position.room, position}, *this );
            setParent( particle, node.m_state.position.room->node );
            m_particles.emplace_back( particle );
        }
    }
}

void Level::finishLevelEffect()
{
    m_levelFinished = true;
}

void Level::earthquakeEffect()
{
    switch( m_effectTimer )
    {
        case 0:
            playSound( 99, boost::none );
            m_cameraController->setBounce( -250 );
            break;
        case 3:
            playSound( 147, boost::none );
            break;
        case 35:
            playSound( 99, boost::none );
            break;
        case 20:
        case 50:
        case 70:
            playSound( 70, boost::none );
            break;
    }

    ++m_effectTimer;
    if( m_effectTimer == 105 )
    {
        m_activeEffect.reset();
    }
}

void Level::floodEffect()
{
    if( m_effectTimer <= 120 )
    {
        auto pos = m_lara->m_state.position.position;
        int mul;
        if( m_effectTimer >= 30 )
        {
            mul = m_effectTimer - 30;
        }
        else
        {
            mul = 30 - m_effectTimer;
        }
        pos.Y = 100 * mul + m_cameraController->getCenter().position.Y;
        playSound( 81, pos.toRenderSystem() );
    }
    else
    {
        m_activeEffect.reset();
    }
    ++m_effectTimer;
}

void Level::chandelierEffect()
{
    playSound( 117, boost::none );
    m_activeEffect.reset();
}

void Level::raisingBlockEffect()
{
    if( m_effectTimer++ == 5 )
    {
        playSound( 119, boost::none );
        m_activeEffect.reset();
    }
}

void Level::stairsToSlopeEffect()
{
    if( m_effectTimer <= 120 )
    {
        if( m_effectTimer == 0 )
        {
            playSound( 161, boost::none );
        }
        auto pos = m_cameraController->getCenter().position;
        pos.Y += 100 * m_effectTimer;
        playSound( 118, pos.toRenderSystem() );
    }
    else
    {
        m_activeEffect.reset();
    }
    ++m_effectTimer;
}

void Level::sandEffect()
{
    if( m_effectTimer <= 120 )
    {
        auto pos = m_cameraController->getCenter().position;
        pos.Y += 100 * m_effectTimer;
        playSound( 155, pos.toRenderSystem() );
    }
    else
    {
        m_activeEffect.reset();
    }
    ++m_effectTimer;
}

void Level::explosionEffect()
{
    playSound( 170, boost::none );
    m_cameraController->setBounce( -75 );
    m_activeEffect.reset();
}

void Level::laraHandsFreeEffect()
{
    m_lara->setHandStatus( engine::HandStatus::None );
}

void Level::flipMapEffect()
{
    swapAllRooms();
}

void Level::unholsterRightGunEffect(engine::items::ItemNode& node)
{
    const auto& src = *m_animatedModels[engine::TR1ItemId::LaraPistolsAnim];
    BOOST_ASSERT( src.models.size() == node.getNode()->getChildren().size() );
    node.getNode()->getChild( 10 )->setDrawable( src.models[10].get() );
}

void Level::chainBlockEffect()
{
    if( m_effectTimer == 0 )
    {
        playSound( 173, boost::none );
    }
    ++m_effectTimer;
    if( m_effectTimer == 55 )
    {
        playSound( 33, boost::none );
        m_activeEffect.reset();
    }
}

void Level::flickerEffect()
{
    if( m_effectTimer > 125 )
    {
        swapAllRooms();
        m_activeEffect.reset();
    }
    else if( m_effectTimer == 90 || m_effectTimer == 92 || m_effectTimer == 105 || m_effectTimer == 107 )
    {
        swapAllRooms();
    }
    ++m_effectTimer;
}

void Level::swapWithAlternate(loader::Room& orig, loader::Room& alternate)
{
    // find any blocks in the original room and un-patch the floor heights

    for( const std::shared_ptr<engine::items::ItemNode>& item : m_itemNodes | boost::adaptors::map_values )
    {
        if( item->m_state.position.room != &orig )
            continue;

        if( const auto tmp = std::dynamic_pointer_cast<engine::items::Block>( item ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, loader::SectorSize );
        }
        else if( const auto tmp = std::dynamic_pointer_cast<engine::items::TallBlock>( item ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, loader::SectorSize * 2 );
        }
    }

    // now swap the rooms and patch the alternate room ids
    std::swap( orig, alternate );
    orig.alternateRoom = alternate.alternateRoom;
    alternate.alternateRoom = -1;

    // move all items over
    swapChildren( gsl::make_not_null( orig.node ), gsl::make_not_null( alternate.node ) );

    // patch heights in the new room, and swap item ownerships.
    // note that this is exactly the same code as above,
    // except for the heights.
    for( const std::shared_ptr<engine::items::ItemNode>& item : m_itemNodes | boost::adaptors::map_values )
    {
        if( item->m_state.position.room == &orig )
        {
            item->m_state.position.room = gsl::make_not_null( &alternate );
        }
        else if( item->m_state.position.room == &alternate )
        {
            item->m_state.position.room = gsl::make_not_null( &orig );
            continue;
        }
        else
        {
            continue;
        }

        if( const auto tmp = std::dynamic_pointer_cast<engine::items::Block>( item ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, -loader::SectorSize );
        }
        else if( const auto tmp = std::dynamic_pointer_cast<engine::items::TallBlock>( item ) )
        {
            loader::Room::patchHeightsForBlock( *tmp, -loader::SectorSize * 2 );
        }
    }

    for( const std::shared_ptr<engine::items::ItemNode>& item : m_dynamicItems )
    {
        if( item->m_state.position.room == &orig )
        {
            item->m_state.position.room = gsl::make_not_null( &alternate );
        }
        else if( item->m_state.position.room == &alternate )
        {
            item->m_state.position.room = gsl::make_not_null( &orig );
        }
    }
}

void Level::addInventoryItem(engine::TR1ItemId id, size_t quantity)
{
    BOOST_LOG_TRIVIAL( debug ) << "Item " << toString( id ) << " added to inventory";

    switch( id )
    {
        case engine::TR1ItemId::PistolsSprite:
        case engine::TR1ItemId::Pistols:
            m_inventory[engine::TR1ItemId::Pistols] += quantity;
            break;
        case engine::TR1ItemId::ShotgunSprite:
        case engine::TR1ItemId::Shotgun:
            if( auto clips = countInventoryItem( engine::TR1ItemId::ShotgunAmmoSprite ) )
            {
                takeInventoryItem( engine::TR1ItemId::ShotgunAmmoSprite, clips );
                m_lara->shotgunAmmo.ammo += 12 * clips;
            }
            m_lara->shotgunAmmo.ammo += 12 * quantity;
            // TODO replaceItems( ShotgunSprite, ShotgunAmmoSprite );
            m_inventory[engine::TR1ItemId::Shotgun] = 1;
            break;
        case engine::TR1ItemId::MagnumsSprite:
        case engine::TR1ItemId::Magnums:
            if( auto clips = countInventoryItem( engine::TR1ItemId::MagnumAmmoSprite ) )
            {
                takeInventoryItem( engine::TR1ItemId::MagnumAmmoSprite, clips );
                m_lara->revolverAmmo.ammo += 50 * clips;
            }
            m_lara->revolverAmmo.ammo += 50 * quantity;
            // TODO replaceItems( MagnumsSprite, MagnumAmmoSprite );
            m_inventory[engine::TR1ItemId::Magnums] = 1;
            break;
        case engine::TR1ItemId::UzisSprite:
        case engine::TR1ItemId::Uzis:
            if( auto clips = countInventoryItem( engine::TR1ItemId::UziAmmoSprite ) )
            {
                takeInventoryItem( engine::TR1ItemId::UziAmmoSprite, clips );
                m_lara->uziAmmo.ammo += 100 * clips;
            }
            m_lara->uziAmmo.ammo += 100 * quantity;
            // TODO replaceItems( UzisSprite, UziAmmoSprite );
            m_inventory[engine::TR1ItemId::Uzis] = 1;
            break;
        case engine::TR1ItemId::ShotgunAmmoSprite:
        case engine::TR1ItemId::ShotgunAmmo:
            if( countInventoryItem( engine::TR1ItemId::ShotgunSprite ) > 0 )
                m_lara->shotgunAmmo.ammo += 12;
            else
                m_inventory[engine::TR1ItemId::ShotgunAmmo] += quantity;
            break;
        case engine::TR1ItemId::MagnumAmmoSprite:
        case engine::TR1ItemId::MagnumAmmo:
            if( countInventoryItem( engine::TR1ItemId::MagnumsSprite ) > 0 )
                m_lara->revolverAmmo.ammo += 50;
            else
                m_inventory[engine::TR1ItemId::MagnumAmmo] += quantity;
            break;
        case engine::TR1ItemId::UziAmmoSprite:
        case engine::TR1ItemId::UziAmmo:
            if( countInventoryItem( engine::TR1ItemId::UzisSprite ) > 0 )
                m_lara->uziAmmo.ammo += 100;
            else
                m_inventory[engine::TR1ItemId::UziAmmo] += quantity;
            break;
        case engine::TR1ItemId::SmallMedipackSprite:
        case engine::TR1ItemId::SmallMedipack:
            m_inventory[engine::TR1ItemId::SmallMedipack] += quantity;
            break;
        case engine::TR1ItemId::LargeMedipackSprite:
        case engine::TR1ItemId::LargeMedipack:
            m_inventory[engine::TR1ItemId::LargeMedipack] += quantity;
            break;
        case engine::TR1ItemId::Puzzle1Sprite:
        case engine::TR1ItemId::Puzzle1:
            m_inventory[engine::TR1ItemId::Puzzle1] += quantity;
            break;
        case engine::TR1ItemId::Puzzle2Sprite:
        case engine::TR1ItemId::Puzzle2:
            m_inventory[engine::TR1ItemId::Puzzle2] += quantity;
            break;
        case engine::TR1ItemId::Puzzle3Sprite:
        case engine::TR1ItemId::Puzzle3:
            m_inventory[engine::TR1ItemId::Puzzle3] += quantity;
            break;
        case engine::TR1ItemId::Puzzle4Sprite:
        case engine::TR1ItemId::Puzzle4:
            m_inventory[engine::TR1ItemId::Puzzle4] += quantity;
            break;
        case engine::TR1ItemId::LeadBarSprite:
        case engine::TR1ItemId::LeadBar:
            m_inventory[engine::TR1ItemId::LeadBar] += quantity;
            break;
        case engine::TR1ItemId::Key1Sprite:
        case engine::TR1ItemId::Key1:
            m_inventory[engine::TR1ItemId::Key1] += quantity;
            break;
        case engine::TR1ItemId::Key2Sprite:
        case engine::TR1ItemId::Key2:
            m_inventory[engine::TR1ItemId::Key2] += quantity;
            break;
        case engine::TR1ItemId::Key3Sprite:
        case engine::TR1ItemId::Key3:
            m_inventory[engine::TR1ItemId::Key3] += quantity;
            break;
        case engine::TR1ItemId::Key4Sprite:
        case engine::TR1ItemId::Key4:
            m_inventory[engine::TR1ItemId::Key4] += quantity;
            break;
        case engine::TR1ItemId::Item141:
        case engine::TR1ItemId::Item148:
            m_inventory[engine::TR1ItemId::Item148] += quantity;
            break;
        case engine::TR1ItemId::Item142:
        case engine::TR1ItemId::Item149:
            m_inventory[engine::TR1ItemId::Item149] += quantity;
            break;
        case engine::TR1ItemId::ScionPiece:
        case engine::TR1ItemId::Item144:
        case engine::TR1ItemId::ScionPiece2:
            m_inventory[engine::TR1ItemId::ScionPiece2] += quantity;
            break;
        default:
            BOOST_LOG_TRIVIAL( warning ) << "Cannot add item " << toString( id ) << " to inventory";
            return;
    }
}

bool Level::tryUseInventoryItem(engine::TR1ItemId id)
{
    if( id == engine::TR1ItemId::Shotgun || id == engine::TR1ItemId::ShotgunSprite )
    {
        if( countInventoryItem( engine::TR1ItemId::Shotgun ) == 0 )
            return false;

        m_lara->requestedGunType = engine::LaraNode::WeaponId::Shotgun;
        if( m_lara->getHandStatus() == engine::HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = engine::LaraNode::WeaponId::None;
        }
    }
    else if( id == engine::TR1ItemId::Pistols || id == engine::TR1ItemId::PistolsSprite )
    {
        if( countInventoryItem( engine::TR1ItemId::Pistols ) == 0 )
            return false;

        m_lara->requestedGunType = engine::LaraNode::WeaponId::Pistols;
        if( m_lara->getHandStatus() == engine::HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = engine::LaraNode::WeaponId::None;
        }
    }
    else if( id == engine::TR1ItemId::Magnums || id == engine::TR1ItemId::MagnumsSprite )
    {
        if( countInventoryItem( engine::TR1ItemId::Magnums ) == 0 )
            return false;

        m_lara->requestedGunType = engine::LaraNode::WeaponId::AutoPistols;
        if( m_lara->getHandStatus() == engine::HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = engine::LaraNode::WeaponId::None;
        }
    }
    else if( id == engine::TR1ItemId::Uzis || id == engine::TR1ItemId::UzisSprite )
    {
        if( countInventoryItem( engine::TR1ItemId::Uzis ) == 0 )
            return false;

        m_lara->requestedGunType = engine::LaraNode::WeaponId::Uzi;
        if( m_lara->getHandStatus() == engine::HandStatus::None && m_lara->gunType == m_lara->requestedGunType )
        {
            m_lara->gunType = engine::LaraNode::WeaponId::None;
        }
    }
    else if( id == engine::TR1ItemId::LargeMedipack || id == engine::TR1ItemId::LargeMedipackSprite )
    {
        if( countInventoryItem( engine::TR1ItemId::LargeMedipack ) == 0 )
            return false;

        if( m_lara->m_state.health <= 0 || m_lara->m_state.health >= core::LaraHealth )
        {
            return false;
        }

        m_lara->m_state.health += 1000;
        if( m_lara->m_state.health > core::LaraHealth )
        {
            m_lara->m_state.health = core::LaraHealth;
        }
        takeInventoryItem( engine::TR1ItemId::LargeMedipackSprite );
        playSound( 116, m_lara->m_state.position.position.toRenderSystem() );
    }
    else if( id == engine::TR1ItemId::SmallMedipack || id == engine::TR1ItemId::SmallMedipackSprite )
    {
        if( countInventoryItem( engine::TR1ItemId::SmallMedipack ) == 0 )
            return false;

        if( m_lara->m_state.health <= 0 || m_lara->m_state.health >= core::LaraHealth )
        {
            return false;
        }

        m_lara->m_state.health += 500;
        if( m_lara->m_state.health > core::LaraHealth )
        {
            m_lara->m_state.health = core::LaraHealth;
        }
        takeInventoryItem( engine::TR1ItemId::SmallMedipackSprite );
        playSound( 116, m_lara->m_state.position.position.toRenderSystem() );
    }

    return true;
}

void Level::animateUV()
{
    static constexpr auto UVAnimTime = 10;

    ++m_uvAnimTime;
    if( m_uvAnimTime >= UVAnimTime )
    {
        m_textureAnimator->updateCoordinates( m_textureProxies );
        m_uvAnimTime -= UVAnimTime;
    }
}

YAML::Node Level::save() const
{
    YAML::Node result;

    YAML::Node inventory;

    const auto addInventory = [&](engine::TR1ItemId id) {
        inventory[engine::toString( id )] = countInventoryItem( id );
    };

    addInventory( engine::TR1ItemId::Pistols );
    addInventory( engine::TR1ItemId::Magnums );
    addInventory( engine::TR1ItemId::MagnumAmmo );
    addInventory( engine::TR1ItemId::Uzis );
    addInventory( engine::TR1ItemId::UziAmmo );
    addInventory( engine::TR1ItemId::Shotgun );
    addInventory( engine::TR1ItemId::ShotgunAmmo );
    addInventory( engine::TR1ItemId::SmallMedipack );
    addInventory( engine::TR1ItemId::LargeMedipack );
    addInventory( engine::TR1ItemId::ScionPiece );
    addInventory( engine::TR1ItemId::Item141 );
    addInventory( engine::TR1ItemId::Item142 );
    addInventory( engine::TR1ItemId::Item142 );
    addInventory( engine::TR1ItemId::Puzzle1 );
    addInventory( engine::TR1ItemId::Puzzle2 );
    addInventory( engine::TR1ItemId::Puzzle3 );
    addInventory( engine::TR1ItemId::Puzzle4 );
    addInventory( engine::TR1ItemId::Key1 );
    addInventory( engine::TR1ItemId::Key2 );
    addInventory( engine::TR1ItemId::Key3 );
    addInventory( engine::TR1ItemId::Key4 );
    addInventory( engine::TR1ItemId::LeadBar );

    YAML::Node lara = m_lara->save();
    lara["inventory"] = inventory;
    result["lara"] = lara;

    YAML::Node flipStatus;
    for( const auto& state : mapFlipActivationStates )
    {
        flipStatus.push_back( state.save() );
    }
    result["flipStatus"] = flipStatus;

    YAML::Node cameraFlags;
    for( const auto& camera : m_cameras )
    {
        cameraFlags.push_back( camera.flags );
    }
    result["cameraFlags"] = cameraFlags;

    if( m_activeEffect.is_initialized() )
        result["flipEffect"] = *m_activeEffect;
    result["flipEffectTimer"] = m_effectTimer;

    for( const auto& item : m_itemNodes )
    {
        YAML::Node node;
        node["type"] = engine::toString( item.second->m_state.object_number );
        node["position"] = item.second->savePosition();
        node["animation"] = item.second->saveAnimation();
        node["health"] = item.second->saveHealth();
        node["triggerInfo"] = item.second->saveTriggerInfo();
        result["items"][item.first] = node;
    }

    result["camera"] = m_cameraController->save();

    return result;
}

void Level::load(const YAML::Node& node)
{
    m_inventory.clear();

    for( const auto& entry : node["lara"]["inventory"] )
        addInventoryItem(
                engine::EnumUtil<engine::TR1ItemId>::fromString( entry.first.as<std::string>() ),
                entry.second.as<size_t>() );

    m_lara->load( node["lara"] );

    for( size_t i = 0; i < mapFlipActivationStates.size(); ++i )
    {
        mapFlipActivationStates[i].load( node["flipStatus"][i] );
    }

    if( !node["cameraFlags"].IsSequence() || node["cameraFlags"].size() != m_cameras.size() )
        BOOST_THROW_EXCEPTION( std::domain_error( "Camera flag sequence is invalid" ) );

    for( size_t i = 0; i < m_cameras.size(); ++i )
        m_cameras[i].flags = node["cameraFlags"][i].as<uint16_t>();

    if( !node["flipEffect"].IsDefined() )
        m_activeEffect.reset();
    else
        m_activeEffect = node["flipEffect"].as<size_t>();

    m_effectTimer = node["flipEffectTimer"].as<int>();

    for( const auto& item : m_itemNodes )
    {
        const auto n = node["items"][item.first];
        if( !n.IsDefined() )
            BOOST_THROW_EXCEPTION( std::domain_error( "Item data is not saved" ) );
        if( engine::EnumUtil<engine::TR1ItemId>::fromString( n["type"].as<std::string>() )
            != item.second->m_state.object_number )
            BOOST_THROW_EXCEPTION( std::domain_error( "Item data has wrong type" ) );
        item.second->loadPosition( n["position"] );
        item.second->loadAnimation( n["animation"] );
        item.second->loadHealth( n["health"] );
        item.second->loadTriggerInfo( n["triggerInfo"] );
    }

    m_cameraController->load( node["camera"] );
}
