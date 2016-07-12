#pragma once

#include "engine/cameracontroller.h"
#include "engine/inputhandler.h"
#include "engine/itemcontroller.h"
#include "game.h"
#include "loader/animation.h"
#include "loader/datatypes.h"
#include "loader/item.h"
#include "audio/device.h"
#include "audio/streamsource.h"

#include <memory>
#include <vector>

class EffectHandler;

namespace level
{
    /** \brief A complete TR level.
      *
      * This contains all necessary functions to load a TR level.
      * Some corrections to the data are done, like converting to OpenGLs coordinate system.
      * All indexes are converted, so they can be used directly.
      * Endian conversion is done at the lowest possible layer, most of the time this is in the read_bitxxx functions.
      */
    class Level
    {
    public:
        Level(Game gameVersion, loader::io::SDLReader&& reader)
            : m_gameVersion(gameVersion)
            , m_reader(std::move(reader))
        {
            m_cdTrackTriggerValues.fill(0);
        }

        virtual ~Level();
        const Game m_gameVersion;

        std::vector<loader::DWordTexture> m_textures;
        std::unique_ptr<loader::Palette> m_palette;
        std::vector<loader::Room> m_rooms;
        loader::FloorData m_floorData;
        std::vector<loader::Mesh> m_meshes;
        std::vector<uint32_t> m_meshIndices;
        std::vector<loader::Animation> m_animations;
        std::vector<loader::Transitions> m_transitions;
        std::vector<loader::TransitionCase> m_transitionCases;
        std::vector<int16_t> m_animCommands;
        std::vector<std::unique_ptr<loader::AnimatedModel>> m_animatedModels;
        std::vector<loader::StaticMesh> m_staticMeshes;
        std::vector<loader::TextureLayoutProxy> m_textureProxies;
        std::vector<uint16_t> m_animatedTextures;
        size_t m_animatedTexturesUvCount = 0;
        std::vector<loader::SpriteTexture> m_spriteTextures;
        std::vector<loader::SpriteSequence> m_spriteSequences;
        std::vector<loader::Camera> m_cameras;
        std::vector<loader::FlybyCamera> m_flybyCameras;
        std::vector<loader::SoundSource> m_soundSources;
        std::vector<loader::Box> m_boxes;
        std::vector<uint16_t> m_overlaps;
        std::vector<loader::Zone> m_zones;
        std::vector<loader::Item> m_items;
        std::map<uint16_t, std::unique_ptr<engine::ItemController>> m_itemControllers;
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

        static std::unique_ptr<Level> createLoader(const std::string& filename, Game game_version);
        virtual void load(irr::video::IVideoDriver* drv) = 0;

        loader::StaticMesh* findStaticMeshById(uint32_t object_id);
        const loader::StaticMesh* findStaticMeshById(uint32_t object_id) const;
        loader::Item* findItemByType(int32_t object_id);
        loader::AnimatedModel* findModelByType(uint32_t object_id);
        int findStaticMeshIndexById(uint32_t object_id) const;
        boost::optional<size_t> findAnimatedModelIndexForType(uint32_t object_id) const;
        boost::optional<size_t> findSpriteSequenceForType(uint32_t object_id) const;

        std::vector<irr::video::ITexture*> createTextures(irr::scene::ISceneManager* mgr);
        std::map<loader::TextureLayoutProxy::TextureKey, irr::video::SMaterial> createMaterials(const std::vector<irr::video::ITexture*>& textures);
        engine::LaraController* createItems(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::ISkinnedMesh*>& skinnedMeshes, const std::vector<irr::video::ITexture*>& textures);
        std::vector<irr::scene::ISkinnedMesh*> createSkinnedMeshes(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::SMesh*>& staticMeshes);
        loader::AnimatedModel::FrameRange loadAnimation(irr::u32& frameOffset, const loader::AnimatedModel& model, const loader::Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh);
        irr::video::ITexture* createSolidColorTex(irr::scene::ISceneManager* mgr, uint8_t color) const;

        void toIrrlicht(irr::IrrlichtDevice* device);

        gsl::not_null<const loader::Sector*> findFloorSectorWithClampedPosition(const core::TRCoordinates& position, gsl::not_null<const loader::Room*> room) const
        {
            return findFloorSectorWithClampedPosition(position, &room);
        }

        gsl::not_null<const loader::Sector*> findFloorSectorWithClampedPosition(core::RoomBoundPosition& rbs) const
        {
            return findFloorSectorWithClampedPosition(rbs.position.toInexact(), &rbs.room);
        }

        gsl::not_null<const loader::Sector*> findFloorSectorWithClampedPosition(core::RoomBoundIntPosition& rbs) const
        {
            return findFloorSectorWithClampedPosition(rbs.position, &rbs.room);
        }

        gsl::not_null<const loader::Sector*> findFloorSectorWithClampedPosition(const core::TRCoordinates& position, gsl::not_null<gsl::not_null<const loader::Room*>*> room) const;

        gsl::not_null<const loader::Room*> findRoomForPosition(const core::ExactTRCoordinates& position, gsl::not_null<const loader::Room*> room) const;

        std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::Sector*> sector, const core::TRCoordinates& position) const
        {
            while( sector->roomBelow != 0xff )
            {
                auto room = &m_rooms[sector->roomBelow];
                sector = room->getSectorByAbsolutePosition(position);
            }

            if( position.Y + loader::QuarterSectorSize * 2 < sector->floorHeight * loader::QuarterSectorSize )
                return {0,0};
            if( sector->floorDataIndex == 0 )
                return {0,0};
            if( loader::extractFDFunction(m_floorData[sector->floorDataIndex]) != loader::FDFunction::FloorSlant )
                return {0,0};

            auto fd = m_floorData[sector->floorDataIndex + 1];
            return {gsl::narrow_cast<int8_t>(fd & 0xff), gsl::narrow_cast<int8_t>(fd >> 8)};
        }

        engine::LaraController* m_lara = nullptr;
        std::shared_ptr<render::TextureAnimator> m_textureAnimator;
        std::shared_ptr<EffectHandler> m_fx = nullptr;

        engine::ItemController* getItemController(uint16_t id) const;

        void drawBars(irr::video::IVideoDriver* drv) const;

        engine::ItemController* findControllerForNode(const irr::scene::ISceneNode* node);

        std::unique_ptr<engine::InputHandler> m_inputHandler;

        audio::Device m_audioDev;
        std::map<size_t, std::weak_ptr<audio::SourceHandle>> m_samples;

        std::shared_ptr<audio::SourceHandle> playSample(size_t sample, float pitch, float volume, const core::ExactTRCoordinates& pos)
        {
            Expects(sample < m_sampleIndices.size());
            pitch = irr::core::clamp(pitch, 0.5f, 2.0f);
            volume = irr::core::clamp(volume, 0.0f, 1.0f);

            std::shared_ptr<audio::BufferHandle> buf = std::make_shared<audio::BufferHandle>();
            const auto idx = m_sampleIndices[sample];
            buf->fillFromWav(&m_samplesData[idx]);

            std::shared_ptr<audio::SourceHandle> src = std::make_shared<audio::SourceHandle>();
            src->setBuffer(buf);
            m_audioDev.registerSource(src);
            src->setPitch(pitch);
            src->setGain(volume);
            src->setPosition(pos.toIrrlicht());
            src->play();

            m_samples[sample] = src;

            return src;
        }

        std::shared_ptr<audio::SourceHandle> findSample(size_t sample) const
        {
            auto it = m_samples.find(sample);
            if(it == m_samples.end())
                return nullptr;

            return it->second.lock();
        }

        void playStream(uint16_t trackId);
        void startTrack(uint16_t trackId);
        void stopTrack(uint16_t trackId);
        void triggerTrack(uint16_t trackId, uint16_t triggerArg, loader::TriggerType triggerType);
        void playTrack(uint16_t trackId, uint16_t triggerArg, loader::TriggerType triggerType);
        void stopSound(uint16_t soundId) const
        {
            BOOST_ASSERT(soundId < m_soundmap.size());
            const auto& details = m_soundDetails[m_soundmap[soundId]];
            const size_t first = details.sample;
            const size_t last = first + details.getSampleCount();

            for(size_t i = first; i < last; ++i)
            {
                stopSample(i);
            }
        }
        void stopSample(size_t id) const
        {
            if(auto handle = findSample(id))
                handle->stop();
        }

        std::shared_ptr<audio::Stream> m_cdStream;
        int m_activeCDTrack = 0;

    protected:
        loader::io::SDLReader m_reader;
        bool m_demoOrUb = false;

        void readMeshData(loader::io::SDLReader& reader);
        void readPoseDataAndModels(loader::io::SDLReader& reader);

        static void convertTexture(loader::ByteTexture& tex, loader::Palette& pal, loader::DWordTexture& dst);
        static void convertTexture(loader::WordTexture& tex, loader::DWordTexture& dst);

        void loadAnimFrame(irr::u32 frameIdx, irr::u32 frameOffset, const loader::AnimatedModel& model, const loader::Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh, gsl::not_null<const int16_t*>& pData, irr::core::aabbox3di& bbox);

    private:
        static Game probeVersion(loader::io::SDLReader& reader, const std::string& filename);
        static std::unique_ptr<Level> createLoader(loader::io::SDLReader&& reader, Game game_version, const std::string& sfxPath);

        std::array<uint16_t, 64> m_cdTrackTriggerValues;
        int m_cdTrack50time = 0;
    };
}
