#pragma once

#include "audio/device.h"
#include "audio/streamsource.h"
#include "engine/cameracontroller.h"
#include "engine/inputhandler.h"
#include "engine/particle.h"
#include "engine/items/itemnode.h"
#include "engine/items_tr1.h"
#include "game.h"
#include "loader/animation.h"
#include "loader/datatypes.h"
#include "loader/item.h"
#include "loader/mesh.h"

#include <memory>
#include <vector>
#include <boost/detail/container_fwd.hpp>
#include <boost/filesystem/path.hpp>

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
    Level(Game gameVersion, loader::io::SDLReader&& reader, sol::state&& scriptEngine)
            : m_gameVersion{gameVersion}
            , m_scriptEngine{std::move( scriptEngine )}
            , m_reader{std::move( reader )}
    {
    }

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

    engine::items::ItemList m_itemNodes;

    std::set<std::shared_ptr<engine::items::ItemNode>> m_dynamicItems;

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

    std::vector<int16_t> m_poseData;

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

    engine::CameraController* m_cameraController = nullptr;

    static std::shared_ptr<Level> createLoader(const std::string& filename, Game gameVersion,
                                               sol::state&& scriptEngine);

    virtual void loadFileData() = 0;

    const loader::StaticMesh* findStaticMeshById(uint32_t object_id) const;

    int findStaticMeshIndexById(uint32_t object_id) const;

    const std::unique_ptr<loader::SkeletalModelType>& findAnimatedModelForType(engine::TR1ItemId type) const;

    const std::unique_ptr<loader::SpriteSequence>& findSpriteSequenceForType(engine::TR1ItemId type) const;

    void createTextures(loader::trx::Glidos* glidos, const boost::filesystem::path& lvlName);

    std::map<loader::TextureLayoutProxy::TextureKey, gsl::not_null<std::shared_ptr<gameplay::Material>>>
    createMaterials(const gsl::not_null<std::shared_ptr<gameplay::ShaderProgram>>& shader);

    engine::LaraNode* createItems();

    void setUpRendering(const gsl::not_null<gameplay::Game*>& game,
                        const boost::filesystem::path& assetPath,
                        const boost::filesystem::path& lvlName,
                        const std::unique_ptr<loader::trx::Glidos>& glidos);

    template<typename T>
    std::shared_ptr<engine::items::ItemNode> createItem(engine::TR1ItemId type,
                                                        const gsl::not_null<const loader::Room*>& room,
                                                        const core::Angle& angle,
                                                        const core::TRVec& position,
                                                        uint16_t activationState)
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

        auto node = createSkeletalModel<T>( *model, room, item );

        m_dynamicItems.insert( node );
        addChild( to_not_null( room->node ), to_not_null( node->getNode() ) );

        return node;
    }

    const loader::Sector* findRealFloorSector(const core::TRVec& position,
                                              gsl::not_null<const loader::Room*> room) const
    {
        return findRealFloorSector( position, to_not_null( &room ) );
    }

    const loader::Sector* findRealFloorSector(core::RoomBoundPosition& rbs) const
    {
        return findRealFloorSector( rbs.position, to_not_null( &rbs.room ) );
    }

    const loader::Sector* findRealFloorSector(const core::TRVec& position,
                                              const gsl::not_null<gsl::not_null<const loader::Room*>*>& room) const;

    gsl::not_null<const loader::Room*> findRoomForPosition(const core::TRVec& position,
                                                           gsl::not_null<const loader::Room*> room) const;

    std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::Sector*> sector,
                                                 const core::TRVec& position) const
    {
        while( sector->roomBelow != nullptr )
        {
            sector = to_not_null( sector->roomBelow->getSectorByAbsolutePosition( position ) );
        }

        static const auto zero = std::make_tuple( 0, 0 );

        if( position.Y + loader::QuarterSectorSize * 2 < sector->floorHeight * loader::QuarterSectorSize )
            return zero;
        if( sector->floorData == nullptr )
            return zero;
        if( engine::floordata::FloorDataChunk{*sector->floorData}.type
            != engine::floordata::FloorDataChunkType::FloorSlant )
            return zero;

        auto fd = sector->floorData[1];
        return std::make_tuple( gsl::narrow_cast<int8_t>( fd & 0xff ), gsl::narrow_cast<int8_t>( fd >> 8 ) );
    }

    engine::LaraNode* m_lara = nullptr;

    std::shared_ptr<render::TextureAnimator> m_textureAnimator;

    engine::items::ItemNode* getItemController(uint16_t id) const;

    void drawBars(const gsl::not_null<gameplay::Game*>& game,
                  const gsl::not_null<std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>>>& image) const;

    std::unique_ptr<engine::InputHandler> m_inputHandler;

    audio::Device m_audioDev;

    std::map<size_t, std::weak_ptr<audio::SourceHandle>> m_samples;

    std::shared_ptr<audio::SourceHandle> playSample(size_t sample, float pitch, float volume,
                                                    const boost::optional<glm::vec3>& pos)
    {
        Expects( sample < m_sampleIndices.size() );
        pitch = util::clamp( pitch, 0.5f, 2.0f );
        volume = util::clamp( volume, 0.0f, 1.0f );

        auto buf = m_audioDev.createBuffer();
        const auto offset = m_sampleIndices[sample];
        BOOST_ASSERT( offset < m_samplesData.size() );
        buf->fillFromWav( &m_samplesData[offset] );

        auto src = m_audioDev.createSource();
        src->setBuffer( buf );
        src->setPitch( pitch );
        src->setGain( volume );
        if( pos.is_initialized() )
            src->setPosition( *pos );

        src->play();

        m_samples[sample] = src.get();

        return src;
    }

    std::shared_ptr<audio::SourceHandle> playSound(int id, const boost::optional<glm::vec3>& position)
    {
        Expects( id >= 0 && static_cast<size_t>(id) < m_soundmap.size() );
        auto snd = m_soundmap[id];
        if( snd < 0 )
        {
            BOOST_LOG_TRIVIAL( warning ) << "No mapped sound for id " << id;
            return nullptr;
        }

        BOOST_ASSERT( snd >= 0 && static_cast<size_t>(snd) < m_soundDetails.size() );
        const loader::SoundDetails& details = m_soundDetails[snd];
        if( details.chance != 0 && util::rand15() > details.chance )
            return nullptr;

        size_t sample = details.sample;
        if( details.getSampleCount() > 1 )
            sample += util::rand15( details.getSampleCount() );
        BOOST_ASSERT( sample < m_sampleIndices.size() );

        float pitch = 1;
        if( details.useRandomPitch() )
            pitch = 0.9f + 0.2f * util::rand15() / 0x8000;

        float volume = util::clamp( static_cast<float>(details.volume) / 0x7fff, 0.0f, 1.0f );
        if( details.useRandomVolume() )
            volume -= 0.25f * util::rand15() / 0x8000;
        if( volume <= 0 )
            return nullptr;

        std::shared_ptr<audio::SourceHandle> handle;
        if( details.getPlaybackType( Engine::TR1 ) == loader::PlaybackType::Looping )
        {
            handle = playSample( sample, pitch, volume, position );
            handle->setLooping( true );
        }
        else if( details.getPlaybackType( Engine::TR1 ) == loader::PlaybackType::Restart )
        {
            handle = findSample( sample );
            if( handle != nullptr )
            {
                handle->play();
            }
            else
            {
                handle = playSample( sample, pitch, volume, position );
            }
        }
        else if( details.getPlaybackType( Engine::TR1 ) == loader::PlaybackType::Wait )
        {
            handle = findSample( sample );
            if( handle == nullptr )
            {
                handle = playSample( sample, pitch, volume, position );
            }
        }
        else
        {
            handle = playSample( sample, pitch, volume, position );
        }

        return handle;
    }

    std::shared_ptr<audio::SourceHandle> findSample(size_t sample) const
    {
        auto it = m_samples.find( sample );
        if( it == m_samples.end() || it->second.expired() )
            return nullptr;

        return it->second.lock();
    }

    void playStream(uint16_t trackId);

    void playCdTrack(uint16_t trackId);

    void stopCdTrack(uint16_t trackId);

    void triggerNormalCdTrack(uint16_t trackId, const engine::floordata::ActivationState& activationRequest,
                              engine::floordata::SequenceCondition triggerType);

    void triggerCdTrack(uint16_t trackId, const engine::floordata::ActivationState& activationRequest,
                        engine::floordata::SequenceCondition triggerType);

    void stopSoundEffect(uint16_t soundId) const
    {
        BOOST_ASSERT( soundId < m_soundmap.size() );
        const auto& details = m_soundDetails[m_soundmap[soundId]];
        const size_t first = details.sample;
        const size_t last = first + details.getSampleCount();

        bool anyStopped = false;
        for( size_t i = first; i < last; ++i )
        {
            anyStopped |= stopSample( i );
        }

        if( !anyStopped )
            BOOST_LOG_TRIVIAL( debug ) << "Attempting to stop sound #" << soundId << " (samples " << first << ".."
                                       << (last - 1) << ") didn't stop any samples";
        else
            BOOST_LOG_TRIVIAL( debug ) << "Stopped samples of sound #" << soundId;
    }

    bool stopSample(size_t id) const
    {
        if( auto handle = findSample( id ) )
        {
            handle->stop();
            return true;
        }

        return false;
    }

    std::weak_ptr<audio::Stream> m_cdStream;

    int m_activeCDTrack = 0;

    void useAlternativeLaraAppearance();

    const gsl::not_null<std::shared_ptr<gameplay::Model>>& getModel(size_t idx) const
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

    std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>> m_models2;

    std::shared_ptr<gameplay::Material> m_spriteMaterial{nullptr};

    void turn180Effect(engine::items::ItemNode& node);

    void dinoStompEffect(engine::items::ItemNode& node);

    void laraNormalEffect(engine::items::ItemNode& node);

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

    void runEffect(size_t id, engine::items::ModelItemNode* node)
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
                Expects( node != nullptr );
                return laraNormalEffect( *node );
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

    bool takeInventoryItem(engine::TR1ItemId id, size_t quantity = 1)
    {
        BOOST_LOG_TRIVIAL(debug) << "Taking item " << toString(id) << " from inventory";

        auto it = m_inventory.find( id );
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

    size_t countInventoryItem(engine::TR1ItemId id) const
    {
        auto it = m_inventory.find( id );
        if( it == m_inventory.end() )
            return 0;

        return it->second;
    }

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

    template<typename T>
    std::shared_ptr<T> createSkeletalModel(const loader::SkeletalModelType& model,
                                           const gsl::not_null<const loader::Room*>& room,
                                           const loader::Item& item);

    std::array<engine::floordata::ActivationState, 64> m_cdTrackActivationStates;

    int m_cdTrack50time = 0;

    std::vector<gsl::not_null<std::shared_ptr<gameplay::Model>>> m_models;

    std::map<engine::TR1ItemId, size_t> m_inventory;
};
}
                                                                                                                                                                                                                                           