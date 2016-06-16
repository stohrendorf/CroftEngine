#pragma once

#include "datatypes.h"
#include "game.h"

#include <memory>
#include <vector>

class LaraController;
class CameraController;

namespace loader
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
    Level(Game gameVersion, io::SDLReader&& reader)
        : m_gameVersion(gameVersion)
        , m_reader(std::move(reader))
    {
    }

    virtual ~Level() = default;

    const Game m_gameVersion;

    std::vector<DWordTexture> m_textures;
    std::unique_ptr<Palette> m_palette;
    std::vector<Room> m_rooms;
    FloorData m_floorData;
    std::vector<Mesh> m_meshes;
    std::vector<uint32_t> m_meshIndices;
    std::vector<Animation> m_animations;
    std::vector<Transitions> m_transitions;
    std::vector<TransitionCase> m_transitionCases;
    std::vector<int16_t> m_animCommands;
    std::vector<std::unique_ptr<AnimatedModel>> m_animatedModels;
    std::vector<StaticMesh> m_staticMeshes;
    std::vector<UVTexture> m_uvTextures;
    std::vector<uint16_t> m_animatedTextures;
    size_t m_animatedTexturesUvCount = 0;
    std::vector<SpriteTexture> m_spriteTextures;
    std::vector<SpriteSequence> m_spriteSequences;
    std::vector<Camera> m_cameras;
    std::vector<FlybyCamera> m_flybyCameras;
    std::vector<SoundSource> m_soundSources;
    std::vector<Box> m_boxes;
    std::vector<uint16_t> m_overlaps;
    std::vector<Zone> m_zones;
    std::vector<Item> m_items;
    std::unique_ptr<LightMap> m_lightmap;
    std::vector<AIObject> m_aiObjects;
    std::vector<CinematicFrame> m_cinematicFrames;
    std::vector<uint8_t> m_demoData;
    std::vector<int16_t> m_soundmap;
    std::vector<SoundDetails> m_soundDetails;
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

    CameraController* m_cameraController = nullptr;

    static std::unique_ptr<Level> createLoader(const std::string &filename, Game game_version);
    virtual void load(irr::video::IVideoDriver* drv) = 0;

    StaticMesh *findStaticMeshById(uint32_t object_id);
    const StaticMesh *findStaticMeshById(uint32_t object_id) const;
    Item* findItemById(int32_t object_id);
    AnimatedModel* findModelById(uint32_t object_id);
    int findStaticMeshIndexByObjectId(uint32_t object_id) const;
    int findAnimatedModelIndexByObjectId(uint32_t object_id) const;
    int findSpriteSequenceByObjectId(uint32_t object_id) const;
    
    struct PlayerInfo
    {
        irr::scene::IAnimatedMeshSceneNode* node = nullptr;
        Room* room = nullptr;
        LaraController* controller = nullptr;
    };
    
    std::vector<irr::video::ITexture*> createTextures(irr::scene::ISceneManager* mgr);
    std::map<UVTexture::TextureKey, irr::video::SMaterial> createMaterials(const std::vector<irr::video::ITexture*>& textures);
    PlayerInfo createItems(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::ISkinnedMesh*>& skinnedMeshes);
    std::vector<irr::scene::ISkinnedMesh*> createSkinnedMeshes(irr::scene::ISceneManager* mgr, const std::vector<irr::scene::SMesh*>& staticMeshes);
    AnimatedModel::FrameRange loadAnimation(irr::u32 & frameOffset, const AnimatedModel& model, const Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh);
    irr::video::ITexture* createSolidColorTex(irr::scene::ISceneManager* mgr, uint8_t color) const;
    
    void toIrrlicht(irr::scene::ISceneManager* mgr, irr::gui::ICursorControl* cursorCtrl);
    
    AbstractTriggerHandler* findHandler(uint16_t itemId) const
    {
        if(itemId >= m_items.size())
            return nullptr;
        
        return m_items[itemId].triggerHandler.get();
    }
    
    void updateTriggers(irr::f32 frameTime)
    {
        for(Item& item : m_items)
        {
            if(item.triggerHandler)
                item.triggerHandler->update(frameTime);
        }
    }
    
    const Sector* findSectorForPosition(const TRCoordinates& position, const Room* room) const
    {
        return findSectorForPosition(position, &room);
    }

    const Sector* findSectorForPosition(const TRCoordinates& position, const Room** room) const;

    const Room* findRoomForPosition(const TRCoordinates& position, const Room* room) const;

    std::tuple<int8_t,int8_t> getFloorSlantInfo(const Sector* sector, const TRCoordinates& position) const
    {
        BOOST_ASSERT(sector != nullptr);
        while(sector->roomBelow != 0xff)
        {
            auto room = &m_rooms[sector->roomBelow];
            sector = room->getSectorByAbsolutePosition(position);
            BOOST_ASSERT(sector != nullptr);
        }

        if(position.Y + QuarterSectorSize * 2 < sector->floorHeight*QuarterSectorSize)
            return {0,0};
        if(sector->floorDataIndex == 0)
            return {0,0};
        if(extractFDFunction(m_floorData[sector->floorDataIndex]) != FDFunction::FloorSlant)
            return {0,0};

        auto fd = m_floorData[sector->floorDataIndex + 1];
        return { static_cast<int8_t>(fd & 0xff), static_cast<int8_t>(fd >> 8) };
    }

    irr::scene::IAnimatedMeshSceneNode* m_lara = nullptr;

protected:
    io::SDLReader m_reader;
    bool m_demoOrUb = false;

    void readMeshData(io::SDLReader& reader);
    void readPoseDataAndModels(io::SDLReader& reader);

    static void convertTexture(ByteTexture & tex, Palette & pal, DWordTexture & dst);
    static void convertTexture(WordTexture & tex, DWordTexture & dst);

    void loadAnimFrame(irr::u32 frameIdx, irr::f32 frameOffset, const AnimatedModel& model, const Animation& animation, irr::scene::ISkinnedMesh* skinnedMesh, const int16_t*& pData, irr::core::aabbox3di& bbox);

private:
    static Game probeVersion(io::SDLReader& reader, const std::string &filename);
    static std::unique_ptr<Level> createLoader(io::SDLReader&& reader, Game game_version, const std::string& sfxPath);
};
}
