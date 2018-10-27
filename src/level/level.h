#pragma once

#include "audio/device.h"
#include "audio/streamsource.h"
#include "engine/cameracontroller.h"
#include "engine/inputhandler.h"
#include "engine/particle.h"
#include "engine/items/itemnode.h"
#include "engine/items/pickupitem.h"
#include "engine/items_tr1.h"
#include "engine/sounds_tr1.h"
#include "engine/tracks_tr1.h"
#include "game.h"
#include "loader/animation.h"
#include "loader/datatypes.h"
#include "loader/item.h"
#include "loader/mesh.h"

#include <boost/filesystem/path.hpp>

#include <memory>
#include <vector>

namespace loader
{
namespace trx
{
class Glidos;
}
}

namespace level
{

class Level
{
public:
    Level(const Game gameVersion, loader::io::SDLReader&& reader, sol::state&& scriptEngine)
            : m_gameVersion{gameVersion}
            , m_scriptEngine{std::move( scriptEngine )}
            , m_reader{std::move( reader )}
    {
    }

    Level(const Level&) = delete;

    Level(Level&&) = delete;

    Level& operator=(const Level&) = delete;

    Level& operator=(Level&&) = delete;

    virtual ~Level();

    const Game m_gameVersion;

    sol::state m_scriptEngine;

    std::vector<loader::DWordTexture> m_textures;

    std::unique_ptr<loader::Palette> m_palette;

    std::vector<loader::Room> m_rooms;

    std::vector<uint32_t> m_meshIndices;

    std::vector<loader::Animation> m_animations;

    std::vector<loader::Transitions> m_transitions;

    std::vector<loader::TransitionCase> m_transitionCases;

    std::vector<int16_t> m_animCommands;

    std::map<engine::TR1ItemId, std::unique_ptr<loader::SkeletalModelType>> m_animatedModels;

    std::vector<loader::TextureLayoutProxy> m_textureProxies;

    size_t m_animatedTexturesUvCount = 0;

    std::vector<loader::Sprite> m_sprites;

    std::map<engine::TR1ItemId, std::unique_ptr<loader::SpriteSequence>> m_spriteSequences;

    std::vector<loader::Camera> m_cameras;

    std::vector<loader::FlybyCamera> m_flybyCameras;

    std::vector<loader::SoundSource> m_soundSources;

    std::vector<loader::Box> m_boxes;

    std::vector<uint16_t> m_overlaps;

    loader::Zones m_baseZones;

    loader::Zones m_alternateZones;

    std::vector<loader::Item> m_items;

    std::map<uint16_t, gsl::not_null<std::shared_ptr<engine::items::ItemNode>>> m_itemNodes;

    std::set<gsl::not_null<std::shared_ptr<engine::items::ItemNode>>> m_dynamicItems;

    std::set<engine::items::ItemNode*> m_scheduledDeletions;

    std::unique_ptr<loader::LightMap> m_lightmap;

    std::vector<loader::AIObject> m_aiObjects;

    std::vector<loader::CinematicFrame> m_cinematicFrames;

    std::vector<uint8_t> m_demoData;

    std::vector<int16_t> m_soundmap;

    std::vector<loader::SoundDetails> m_soundDetails;

    size_t m_samplesCount = 0;

    std::vector<uint8_t> m_samplesData;

    std::vector<uint32_t> m_sampleIndices;

    std::vector<int16_t> m_poseFrames;

    std::vector<int32_t> m_boneTrees;

    std::string m_sfxPath = "MAIN.SFX";

    /*
     * 0 Normal
     * 3 Catsuit
     * 4 Divesuit
     * 6 Invisible
     */
    uint16_t m_laraType = 0;

    /*
     * 0 No weather
     * 1 Rain
     * 2 Snow (in title.trc these are red triangles falling from the sky).
     */
    uint16_t m_weatherType = 0;

    std::unique_ptr<engine::CameraController> m_cameraController = nullptr;

    static std::shared_ptr<Level> createLoader(const std::string& filename, Game gameVersion,
                                               sol::state&& scriptEngine);

    virtual void loadFileData() = 0;

    const loader::StaticMesh* findStaticMeshById(uint32_t meshId) const;

    int findStaticMeshIndexById(uint32_t meshId) const;

    const std::unique_ptr<loader::SkeletalModelType>& findAnimatedModelForType(engine::TR1ItemId type) const;

    const std::unique_ptr<loader::SpriteSequence>& findSpriteSequenceForType(engine::TR1ItemId type) const;

    std::map<loader::TextureLayoutProxy::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>
    createMaterials(const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader);

    std::shared_ptr<engine::LaraNode> createItems();

    void setUpRendering(const gsl::not_null<gameplay::Game*>& game);

    template<typename T>
    std::shared_ptr<engine::items::ItemNode> createItem(const engine::TR1ItemId type,
                                                        const gsl::not_null<const loader::Room*>& room,
                                                        const core::Angle& angle,
                                                        const core::TRVec& position,
                                                        const uint16_t activationState)
    {
        const auto& model = findAnimatedModelForType( type );
        if( model == nullptr )
            return nullptr;

        loader::Item item;
        item.type = type;
        item.room = -1;
        item.position = position;
        item.rotation = angle.toAU();
        item.darkness = 0;
        item.activationState = activationState;

        auto node = make_not_null_shared<T>( gsl::make_not_null( this ), room, item, *model );

        m_dynamicItems.emplace( node );
        addChild( gsl::make_not_null( room->node ), gsl::make_not_null( node->getNode() ) );

        return node.get();
    }

    std::shared_ptr<engine::items::PickupItem> createPickup(const engine::TR1ItemId type,
                                                            const gsl::not_null<const loader::Room*>& room,
                                                            const core::TRVec& position);

    static const loader::Sector* findRealFloorSector(const core::TRVec& position,
                                                     gsl::not_null<const loader::Room*> room)
    {
        return findRealFloorSector( position, make_not_null( &room ) );
    }

    static const loader::Sector* findRealFloorSector(core::RoomBoundPosition& rbs)
    {
        return findRealFloorSector( rbs.position, make_not_null( &rbs.room ) );
    }

    static const loader::Sector* findRealFloorSector(const core::TRVec& position,
                                                     const gsl::not_null<gsl::not_null<const loader::Room*>*>& room);

    gsl::not_null<const loader::Room*> findRoomForPosition(const core::TRVec& position,
                                                           gsl::not_null<const loader::Room*> room) const;

    std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::Sector*> sector,
                                                 const core::TRVec& position) const
    {
        while( sector->roomBelow != nullptr )
        {
            sector = gsl::make_not_null( sector->roomBelow->getSectorByAbsolutePosition( position ) );
        }

        static const auto zero = std::make_tuple( 0, 0 );

        if( position.Y + loader::QuarterSectorSize * 2 < sector->floorHeight * loader::QuarterSectorSize )
            return zero;
        if( sector->floorData == nullptr )
            return zero;
        if( engine::floordata::FloorDataChunk{*sector->floorData}.type
            != engine::floordata::FloorDataChunkType::FloorSlant )
            return zero;

        const auto fd = sector->floorData[1];
        return std::make_tuple( gsl::narrow_cast<int8_t>( fd & 0xff ), gsl::narrow_cast<int8_t>( fd >> 8 ) );
    }

    std::shared_ptr<engine::LaraNode> m_lara = nullptr;

    std::shared_ptr<render::TextureAnimator> m_textureAnimator;

    std::shared_ptr<engine::items::ItemNode> getItem(uint16_t id) const;

    void drawBars(const gsl::not_null<gameplay::Game*>& game,
                  const gsl::not_null<std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>>>& image) const;

    std::unique_ptr<engine::InputHandler> m_inputHandler;

    audio::Device m_audioDev;

    std::map<size_t, std::weak_ptr<audio::SourceHandle>> m_samples;

    gsl::not_null<std::shared_ptr<audio::SourceHandle>> playSample(
            const size_t sample,
            const float pitch,
            const float volume,
            const boost::optional<glm::vec3>& pos);

    std::shared_ptr<audio::SourceHandle> playSound(const engine::TR1SoundId id,
                                                   const boost::optional<glm::vec3>& position);

    std::shared_ptr<audio::SourceHandle> findSample(const size_t sample) const
    {
        const auto it = m_samples.find( sample );
        if( it == m_samples.end() )
            return nullptr;

        return it->second.lock();
    }

    gsl::not_null<std::shared_ptr<audio::Stream>> playStream(size_t trackId);

    void playStopCdTrack(engine::TR1TrackId trackId, bool stop);

    void triggerNormalCdTrack(engine::TR1TrackId trackId,
                              const engine::floordata::ActivationState& activationRequest,
                              engine::floordata::SequenceCondition triggerType);

    void triggerCdTrack(engine::TR1TrackId trackId,
                        const engine::floordata::ActivationState& activationRequest,
                        engine::floordata::SequenceCondition triggerType);

    void stopSound(const engine::TR1SoundId soundId) const;

    bool stopSample(const size_t id) const
    {
        if( auto handle = findSample( id ) )
        {
            handle->stop();
            return true;
        }

        return false;
    }

    std::weak_ptr<audio::Stream> m_ambientStream;
    std::weak_ptr<audio::Stream> m_interceptStream;
    boost::optional<engine::TR1TrackId> m_currentTrack;
    boost::optional<engine::TR1SoundId> m_currentLaraTalk;

    void useAlternativeLaraAppearance(bool withHead = false);

    const gsl::not_null<std::shared_ptr<gameplay::Model>>& getModel(const size_t idx) const
    {
        Expects( idx < m_models.size() );

        return m_models[idx];
    }

    void scheduleDeletion(engine::items::ItemNode* item)
    {
        m_scheduledDeletions.insert( item );
    }

    void applyScheduledDeletions()
    {
        if( m_scheduledDeletions.empty() )
            return;

        for( const auto& del : m_scheduledDeletions )
        {
            auto it = std::find_if( m_dynamicItems.begin(), m_dynamicItems.end(),
                                    [del](const std::shared_ptr<engine::items::ItemNode>& i) {
                                        return i.get() == del;
                                    } );
            if( it == m_dynamicItems.end() )
                continue;
            m_dynamicItems.erase( it );
        }

        m_scheduledDeletions.clear();
    }

    bool roomsAreSwapped = false;

    std::vector<gsl::not_null<std::shared_ptr<engine::Particle>>> m_particles;

    // list of meshes and models, resolved through m_meshIndices
    std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>> m_modelsDirect;
    std::vector<gsl::not_null<const loader::Mesh*>> m_meshesDirect;

    std::shared_ptr<gameplay::Material> m_spriteMaterial{nullptr};

    void turn180Effect(engine::items::ItemNode& node);

    void dinoStompEffect(engine::items::ItemNode& node);

    void laraNormalEffect();

    void laraBubblesEffect(engine::items::ItemNode& node);

    void finishLevelEffect();

    void earthquakeEffect();

    void floodEffect();

    void chandelierEffect();

    void raisingBlockEffect();

    void stairsToSlopeEffect();

    void sandEffect();

    void explosionEffect();

    void laraHandsFreeEffect();

    void flipMapEffect();

    void unholsterRightGunEffect(engine::items::ItemNode& node);

    void chainBlockEffect();

    void flickerEffect();

    void swapAllRooms()
    {
        BOOST_LOG_TRIVIAL( info ) << "Swapping rooms";
        for( auto& room : m_rooms )
        {
            if( room.alternateRoom < 0 )
                continue;

            BOOST_ASSERT( static_cast<size_t>(room.alternateRoom) < m_rooms.size() );
            swapWithAlternate( room, m_rooms[room.alternateRoom] );
        }

        roomsAreSwapped = !roomsAreSwapped;
    }

    void setGlobalEffect(size_t fx)
    {
        m_activeEffect = fx;
        m_effectTimer = 0;
    }

    void doGlobalEffect()
    {
        if( !m_activeEffect.is_initialized() )
            return;

        runEffect( *m_activeEffect, nullptr );
    }

    void runEffect(const size_t id, engine::items::ModelItemNode* node)
    {
        switch( id )
        {
            case 0:
                Expects( node != nullptr );
                return turn180Effect( *node );
            case 1:
                Expects( node != nullptr );
                return dinoStompEffect( *node );
            case 2:
                return laraNormalEffect();
            case 3:
                Expects( node != nullptr );
                return laraBubblesEffect( *node );
            case 4:
                return finishLevelEffect();
            case 5:
                return earthquakeEffect();
            case 6:
                return floodEffect();
            case 7:
                return chandelierEffect();
            case 8:
                return raisingBlockEffect();
            case 9:
                return stairsToSlopeEffect();
            case 10:
                return sandEffect();
            case 11:
                return explosionEffect();
            case 12:
                return laraHandsFreeEffect();
            case 13:
                return flipMapEffect();
            case 14:
                Expects( node != nullptr );
                return unholsterRightGunEffect( *node );
            case 15:
                return chainBlockEffect();
            case 16:
                return flickerEffect();
            default:
                BOOST_LOG_TRIVIAL( warning ) << "Unhandled effect: " << id;
        }
    }

    bool m_levelFinished = false;

    void swapWithAlternate(loader::Room& orig, loader::Room& alternate);

    void addInventoryItem(engine::TR1ItemId id, size_t quantity = 1);

    bool takeInventoryItem(const engine::TR1ItemId id, const size_t quantity = 1)
    {
        BOOST_LOG_TRIVIAL( debug ) << "Taking item " << toString( id ) << " from inventory";

        const auto it = m_inventory.find( id );
        if( it == m_inventory.end() )
            return false;

        if( it->second < quantity )
            return false;

        if( it->second == quantity )
            m_inventory.erase( it );
        else
            m_inventory[id] -= quantity;

        return true;
    }

    bool tryUseInventoryItem(engine::TR1ItemId id);

    size_t countInventoryItem(const engine::TR1ItemId id) const
    {
        const auto it = m_inventory.find( id );
        if( it == m_inventory.end() )
            return 0;

        return it->second;
    }

    void animateUV();

    YAML::Node save() const;

    void load(const YAML::Node& node);

    size_t indexOfModel(const std::shared_ptr<gameplay::Drawable>& m) const
    {
        for( size_t i = 0; i < m_models.size(); ++i )
            if( m_models[i].get() == m )
                return i;

        BOOST_THROW_EXCEPTION( std::runtime_error( "Model not found" ) );
    }

    std::array<engine::floordata::ActivationState, 10> mapFlipActivationStates{};

    engine::items::ItemNode* m_pierre = nullptr;

protected:
    loader::io::SDLReader m_reader;
    engine::floordata::FloorData m_floorData;
    std::vector<loader::Mesh> m_meshes;
    std::vector<loader::StaticMesh> m_staticMeshes;
    std::vector<uint16_t> m_animatedTextures;

    bool m_demoOrUb = false;

    int m_effectTimer = 0;
    boost::optional<size_t> m_activeEffect{};

    void readMeshData(loader::io::SDLReader& reader);

    static void convertTexture(loader::ByteTexture& tex, loader::Palette& pal, loader::DWordTexture& dst);

    static void convertTexture(loader::WordTexture& tex, loader::DWordTexture& dst);

    void postProcessDataStructures();

private:
    static Game probeVersion(loader::io::SDLReader& reader, const std::string& filename);

    static std::shared_ptr<Level>
    createLoader(loader::io::SDLReader&& reader, Game game_version, const std::string& sfxPath,
                 sol::state&& scriptEngine);

    std::array<engine::floordata::ActivationState, static_cast<size_t>(engine::TR1TrackId::Sentinel)> m_cdTrackActivationStates;

    int m_cdTrack50time = 0;

    std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>> m_models;

    std::map<engine::TR1ItemId, size_t> m_inventory;

    int m_uvAnimTime{0};
};
}