#pragma once

#include "audio/device.h"
#include "audio/streamsource.h"
#include "engine/cameracontroller.h"
#include "engine/inputhandler.h"
#include "engine/items/itemnode.h"
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
    engine::floordata::FloorData m_floorData;
    std::vector<loader::Mesh> m_meshes;
    std::vector<uint32_t> m_meshIndices;
    std::vector<loader::Animation> m_animations;
    std::vector<loader::Transitions> m_transitions;
    std::vector<loader::TransitionCase> m_transitionCases;
    std::vector<int16_t> m_animCommands;
    std::map<uint32_t, std::unique_ptr<loader::SkeletalModelType>> m_animatedModels;
    std::vector<loader::StaticMesh> m_staticMeshes;
    std::vector<loader::TextureLayoutProxy> m_textureProxies;
    std::vector<uint16_t> m_animatedTextures;
    size_t m_animatedTexturesUvCount = 0;
    std::vector<loader::Sprite> m_sprites;
    std::vector<loader::SpriteSequence> m_spriteSequences;
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

    static std::unique_ptr<Level>
    createLoader(const std::string& filename, Game gameVersion, sol::state&& scriptEngine);

    virtual void loadFileData() = 0;

    const loader::StaticMesh* findStaticMeshById(uint32_t object_id) const;

    int findStaticMeshIndexById(uint32_t object_id) const;

    const std::unique_ptr<loader::SkeletalModelType>& findAnimatedModelForType(uint32_t type) const;

    boost::optional<size_t> findSpriteSequenceForType(uint32_t type) const;

    std::vector<std::shared_ptr<gameplay::gl::Texture>>
    createTextures(loader::trx::Glidos* glidos, const boost::filesystem::path& lvlName);

    std::map<loader::TextureLayoutProxy::TextureKey, std::shared_ptr<gameplay::Material>>
    createMaterials(const std::vector<std::shared_ptr<gameplay::gl::Texture>>& textures,
                    const std::shared_ptr<gameplay::ShaderProgram>& shader);

    engine::LaraNode* createItems(const std::vector<std::shared_ptr<gameplay::gl::Texture>>& textures);

    void setUpRendering(gameplay::Game* game,
                        const boost::filesystem::path& assetPath,
                        const boost::filesystem::path& lvlName,
                        const std::unique_ptr<loader::trx::Glidos>& glidos);

    template<typename T>
    std::shared_ptr<engine::items::ItemNode> createItem(uint32_t type,
                                                        const gsl::not_null<const loader::Room*>& room,
                                                        const core::Angle& angle,
                                                        const core::TRCoordinates& position,
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

        auto node = createSkeletalModel<T>( 99999, *model, room, item );

        m_dynamicItems.insert( node );
        room->node->addChild( node->getNode() );

        return node;
    }

    gsl::not_null<const loader::Sector*>
    findRealFloorSector(const core::TRCoordinates& position, const loader::Room* room) const
    {
        return findRealFloorSector( position, &room );
    }

    gsl::not_null<const loader::Sector*> findRealFloorSector(core::RoomBoundPosition& rbs) const
    {
        return findRealFloorSector( rbs.position, &rbs.room );
    }

    gsl::not_null<const loader::Sector*>
    findRealFloorSector(const core::TRCoordinates& position, const loader::Room** room) const;

    gsl::not_null<const loader::Room*>
    findRoomForPosition(const core::TRCoordinates& position, gsl::not_null<const loader::Room*> room) const;

    std::tuple<int8_t, int8_t>
    getFloorSlantInfo(gsl::not_null<const loader::Sector*> sector, const core::TRCoordinates& position) const
    {
        while( sector->roomBelow != nullptr )
        {
            sector = sector->roomBelow->getSectorByAbsolutePosition( position );
        }

        static const auto zero = std::make_tuple( 0, 0 );

        if( position.Y + loader::QuarterSectorSize * 2 < sector->floorHeight * loader::QuarterSectorSize )
            return zero;
        if( sector->floorDataIndex == 0 )
            return zero;
        if( engine::floordata::FloorDataChunk{m_floorData[sector->floorDataIndex]}.type
            != engine::floordata::FloorDataChunkType::FloorSlant )
            return zero;

        auto fd = m_floorData[sector->floorDataIndex + 1];
        return std::make_tuple( gsl::narrow_cast<int8_t>( fd & 0xff ), gsl::narrow_cast<int8_t>( fd >> 8 ) );
    }

    engine::LaraNode* m_lara = nullptr;
    std::shared_ptr<render::TextureAnimator> m_textureAnimator;

    engine::items::ItemNode* getItemController(uint16_t id) const;

    void drawBars(gameplay::Game* game, const std::shared_ptr<gameplay::gl::Image<gameplay::gl::RGBA8>>& image) const;

    std::unique_ptr<engine::InputHandler> m_inputHandler;

    audio::Device m_audioDev;
    std::map<size_t, std::weak_ptr<audio::SourceHandle>> m_samples;

    std::shared_ptr<audio::SourceHandle>
    playSample(size_t sample, float pitch, float volume, const boost::optional<glm::vec3>& pos)
    {
        Expects( sample < m_sampleIndices.size() );
        pitch = util::clamp( pitch, 0.5f, 2.0f );
        volume = util::clamp( volume, 0.0f, 1.0f );

        std::shared_ptr<audio::BufferHandle> buf = std::make_shared<audio::BufferHandle>();
        const auto offset = m_sampleIndices[sample];
        BOOST_ASSERT( offset < m_samplesData.size() );
        buf->fillFromWav( &m_samplesData[offset] );

        std::shared_ptr<audio::SourceHandle> src = std::make_shared<audio::SourceHandle>();
        src->setBuffer( buf );
        src->setPitch( pitch );
        src->setGain( volume );
        if( pos.is_initialized() )
            src->setPosition( *pos );

        src->play();

        m_audioDev.registerSource( src );
        m_samples[sample] = src;

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
        if( details.chance != 0 && (rand() & 0x7fff) > details.chance )
            return nullptr;

        size_t sample = details.sample;
        if( details.getSampleCount() > 1 )
            sample += rand() % details.getSampleCount();
        BOOST_ASSERT( sample < m_sampleIndices.size() );

        float pitch = 1;
        if( details.useRandomPitch() )
            pitch = 0.9f + 0.2f * rand() / RAND_MAX;

        float volume = util::clamp( static_cast<float>(details.volume) / 0x7fff, 0.0f, 1.0f );
        if( details.useRandomVolume() )
            volume -= 0.25f * rand() / RAND_MAX;
        if( volume <= 0 )
            return nullptr;

        std::shared_ptr<audio::SourceHandle> handle;
        if( details.getPlaybackType( level::Engine::TR1 ) == loader::PlaybackType::Looping )
        {
            handle = playSample( sample, pitch, volume, position );
            handle->setLooping( true );
        }
        else if( details.getPlaybackType( level::Engine::TR1 ) == loader::PlaybackType::Restart )
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
        else if( details.getPlaybackType( level::Engine::TR1 ) == loader::PlaybackType::Wait )
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
        if( it == m_samples.end() )
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

    std::shared_ptr<audio::Stream> m_cdStream;
    int m_activeCDTrack = 0;

    void useAlternativeLaraAppearance();

    const std::shared_ptr<gameplay::Model>& getModel(size_t idx) const
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

        for(const auto& del : m_scheduledDeletions)
        {
            auto it = std::find_if(m_dynamicItems.begin(), m_dynamicItems.end(), [del](const std::shared_ptr<engine::items::ItemNode>& i)
            {
                return i.get() == del;
            });
            if (it == m_dynamicItems.end())
                continue;
            m_dynamicItems.erase(it);
        }

        m_scheduledDeletions.clear();
    }

    bool roomsAreSwapped = false;

protected:
    loader::io::SDLReader m_reader;
    bool m_demoOrUb = false;

    void readMeshData(loader::io::SDLReader& reader);

    static void convertTexture(loader::ByteTexture& tex, loader::Palette& pal, loader::DWordTexture& dst);

    static void convertTexture(loader::WordTexture& tex, loader::DWordTexture& dst);

    void postProcessDataStructures();

private:
    static Game probeVersion(loader::io::SDLReader& reader, const std::string& filename);

    static std::unique_ptr<Level>
    createLoader(loader::io::SDLReader&& reader, Game game_version, const std::string& sfxPath,
                 sol::state&& scriptEngine);

    template<typename T>
    std::shared_ptr<T> createSkeletalModel(size_t id,
                                           const loader::SkeletalModelType& model,
                                           const gsl::not_null<const loader::Room*>& room,
                                           const loader::Item& item);

    std::array<engine::floordata::ActivationState, 64> m_cdTrackActivationStates;
    int m_cdTrack50time = 0;
    std::vector<std::shared_ptr<gameplay::Model>> m_models;
};
}
