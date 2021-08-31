#pragma once

#include "animation.h"
#include "atlastile.h"
#include "audio/soundengine.h"
#include "box.h"
#include "camerasink.h"
#include "cinematicframe.h"
#include "engine/controllerbuttons.h"
#include "engine/floordata/floordata.h"
#include "engine/objectmanager.h"
#include "engine/objects/object.h"
#include "loader/file/datatypes.h"
#include "loader/file/item.h"
#include "mesh.h"
#include "room.h"
#include "skeletalmodeltype.h"
#include "sprite.h"
#include "staticmesh.h"
#include "staticsoundeffect.h"
#include "transition.h"
#include "ui/pickupwidget.h"

#include <pybind11/pytypes.h>

namespace gl
{
class CImgWrapper;
}

namespace loader::file
{
enum class AnimationId : uint16_t;
struct AnimFrame;
struct SkeletalModelType;
} // namespace loader::file

namespace loader::trx
{
class Glidos;
}

namespace loader::file::level
{
class Level;
}

namespace render
{
class TextureAnimator;
class MultiTextureAtlas;
} // namespace render

namespace engine::objects
{
class ModelObject;
class PickupObject;
} // namespace engine::objects

namespace engine
{
class Presenter;
class Engine;
class AudioEngine;
struct SavegameInfo;
class CameraController;
class Player;
enum class TR1TrackId : int32_t;
} // namespace engine

namespace engine::world
{
struct Animation;
class RenderMeshData;

class World final
{
public:
  explicit World(Engine& engine,
                 std::unique_ptr<loader::file::level::Level>&& level,
                 std::string title,
                 size_t totalSecrets,
                 const std::optional<TR1TrackId>& track,
                 bool useAlternativeLara,
                 std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
                 std::shared_ptr<Player> player);

  ~World();

  [[nodiscard]] bool roomsAreSwapped() const
  {
    return m_roomsAreSwapped;
  }

  CameraController& getCameraController()
  {
    return *m_cameraController;
  }

  [[nodiscard]] const CameraController& getCameraController() const
  {
    return *m_cameraController;
  }

  ObjectManager& getObjectManager()
  {
    return m_objectManager;
  }

  [[nodiscard]] const ObjectManager& getObjectManager() const
  {
    return m_objectManager;
  }

  void finishLevel()
  {
    m_levelFinished = true;
  }

  [[nodiscard]] bool levelFinished() const
  {
    return m_levelFinished;
  }

  void setGlobalEffect(size_t fx)
  {
    m_activeEffect = fx;
    m_effectTimer = 0_frame;
  }

  template<typename T>
  std::shared_ptr<T> createDynamicObject(const core::TypeId& type,
                                         const gsl::not_null<const Room*>& room,
                                         const core::Angle& angle,
                                         const core::TRVec& position,
                                         const uint16_t activationState)
  {
    const auto& model = findAnimatedModelForType(type);
    if(model == nullptr)
      return nullptr;

    loader::file::Item item;
    item.type = type;
    item.room = uint16_t(-1);
    item.position = position;
    item.rotation = angle;
    item.shade = core::Shade{core::Shade::type{0}};
    item.activationState = activationState;

    auto object
      = std::make_shared<T>(objects::makeObjectName(type.get_as<TR1ItemId>(), m_objectManager.getDynamicObjectCount()),
                            this,
                            room,
                            item,
                            model.get());

    m_objectManager.registerDynamicObject(object);
    addChild(room->node, object->getNode());

    return object;
  }

  template<typename T>
  std::shared_ptr<T> createDynamicObject(const Location& location)
  {
    auto object = std::make_shared<T>(this, location);

    m_objectManager.registerDynamicObject(object);

    return object;
  }

  void swapAllRooms();
  bool isValid(const loader::file::AnimFrame* frame) const;
  void swapWithAlternate(Room& orig, Room& alternate);
  [[nodiscard]] const std::vector<Box>& getBoxes() const;
  [[nodiscard]] const std::vector<Room>& getRooms() const;
  std::vector<Room>& getRooms();
  [[nodiscard]] const StaticMesh* findStaticMeshById(const core::StaticMeshId& meshId) const;
  [[nodiscard]] const std::unique_ptr<SpriteSequence>& findSpriteSequenceForType(const core::TypeId& type) const;
  [[nodiscard]] const Animation& getAnimation(loader::file::AnimationId id) const;
  [[nodiscard]] const std::vector<CinematicFrame>& getCinematicFrames() const;
  [[nodiscard]] const std::vector<int16_t>& getAnimCommands() const;
  void update(bool godMode);
  void dinoStompEffect(objects::Object& object);
  void laraNormalEffect();
  void laraBubblesEffect(objects::Object& object);
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
  void chainBlockEffect();
  void flickerEffect();
  void doGlobalEffect();
  std::shared_ptr<objects::PickupObject>
    createPickup(const core::TypeId& type, const gsl::not_null<const Room*>& room, const core::TRVec& position);
  void useAlternativeLaraAppearance(bool withHead = false);
  void runEffect(size_t id, objects::Object* object);
  [[nodiscard]] const std::unique_ptr<SkeletalModelType>& findAnimatedModelForType(const core::TypeId& type) const;
  [[nodiscard]] const std::vector<Animation>& getAnimations() const;
  [[nodiscard]] const std::vector<int16_t>& getPoseFrames() const;
  [[nodiscard]] gsl::not_null<std::shared_ptr<RenderMeshData>> getRenderMesh(size_t idx) const;
  [[nodiscard]] const std::vector<Mesh>& getMeshes() const;
  void turn180Effect(objects::Object& object);
  void drawRightWeaponEffect(const objects::ModelObject& object);
  [[nodiscard]] const std::array<gl::SRGBA8, 256>& getPalette() const;
  void handleCommandSequence(const floordata::FloorDataValue* floorData, bool fromHeavy);
  core::TypeId find(const SkeletalModelType* model) const;
  core::TypeId find(const Sprite* sprite) const;
  void serialize(const serialization::Serializer<World>& ser);
  void gameLoop(bool godMode, float delayRatio, float blackAlpha);
  bool cinematicLoop();
  void load(const std::optional<size_t>& slot);
  void save(const std::optional<size_t>& slot);
  [[nodiscard]] std::map<size_t, SavegameInfo> getSavedGames() const;
  [[nodiscard]] bool hasSavedGames() const;

  [[nodiscard]] const Presenter& getPresenter() const;
  [[nodiscard]] Presenter& getPresenter();

  [[nodiscard]] auto getPierre() const
  {
    return m_pierre;
  }

  void setPierre(objects::Object* pierre)
  {
    m_pierre = pierre;
  }

  [[nodiscard]] const Engine& getEngine() const
  {
    return m_engine;
  }

  Engine& getEngine()
  {
    return m_engine;
  }

  [[nodiscard]] const AudioEngine& getAudioEngine() const
  {
    return *m_audioEngine;
  }

  AudioEngine& getAudioEngine()
  {
    return *m_audioEngine;
  }

  [[nodiscard]] const std::string& getTitle() const
  {
    return m_title;
  }

  [[nodiscard]] auto getTotalSecrets() const
  {
    return m_totalSecrets;
  }

  [[nodiscard]] const auto& getTextureAnimator() const
  {
    BOOST_ASSERT(m_textureAnimator != nullptr);
    return *m_textureAnimator;
  }

  [[nodiscard]] auto& getTextureAnimator()
  {
    BOOST_ASSERT(m_textureAnimator != nullptr);
    return *m_textureAnimator;
  }

  void addPickupWidget(Sprite sprite)
  {
    m_pickupWidgets.emplace_back(75_frame, std::move(sprite));
  }

  [[nodiscard]] std::optional<std::string> getItemTitle(TR1ItemId id) const;

  auto& getPlayer()
  {
    Expects(m_player != nullptr);
    return *m_player;
  }

  [[nodiscard]] const auto& getPlayer() const
  {
    Expects(m_player != nullptr);
    return *m_player;
  }

  [[nodiscard]] const auto& getPlayerPtr() const
  {
    return m_player;
  }

  [[nodiscard]] const auto& getAtlasTiles() const
  {
    return m_atlasTiles;
  }

  [[nodiscard]] const auto& getSprites() const
  {
    return m_sprites;
  }

  [[nodiscard]] const auto& getFloorData() const
  {
    return m_floorData;
  }

  [[nodiscard]] const auto& getCameraSinks() const
  {
    return m_cameraSinks;
  }

  void drawPerformanceBar(ui::Ui& ui, float delayRatio) const;

  [[nodiscard]] const auto& getControllerLayouts() const
  {
    return m_controllerLayouts;
  }

  [[nodiscard]] auto& getMapFlipActivationStates()
  {
    return m_mapFlipActivationStates;
  }

private:
  void drawPickupWidgets(ui::Ui& ui);

  Engine& m_engine;
  const std::filesystem::path m_levelFilename;

  std::unique_ptr<AudioEngine> m_audioEngine;

  std::unique_ptr<CameraController> m_cameraController;

  core::Frame m_effectTimer = 0_frame;
  std::optional<size_t> m_activeEffect{};
  std::shared_ptr<audio::Voice> m_globalSoundEffect{};

  bool m_roomsAreSwapped = false;

  ObjectManager m_objectManager;

  bool m_levelFinished = false;

  struct PositionalEmitter final : public audio::Emitter
  {
    glm::vec3 position;

    PositionalEmitter(const glm::vec3& position, const gsl::not_null<audio::SoundEngine*>& engine)
        : Emitter{engine}
        , position{position}
    {
    }

    glm::vec3 getPosition() const override
    {
      return position;
    }
  };

  std::vector<PositionalEmitter> m_positionalEmitters;

  std::bitset<16> m_secretsFoundBitmask = 0;

  std::array<floordata::ActivationState, 10> m_mapFlipActivationStates;
  objects::Object* m_pierre = nullptr;
  std::string m_title{};
  size_t m_totalSecrets = 0;
  std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> m_itemTitles{};
  std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>> m_allTextures;
  std::shared_ptr<gl::TextureHandle<gl::Texture2DArray<gl::SRGBA8>>> m_allTexturesHandle;
  core::Frame m_uvAnimTime = 0_frame;
  std::unique_ptr<render::TextureAnimator> m_textureAnimator;

  std::vector<ui::PickupWidget> m_pickupWidgets{};
  const std::shared_ptr<Player> m_player;

  std::vector<int16_t> m_poseFrames;
  std::vector<int16_t> m_animCommands;
  std::vector<int32_t> m_boneTrees;
  engine::floordata::FloorData m_floorData;
  std::array<gl::SRGBA8, 256> m_palette;
  std::vector<uint8_t> m_samplesData;

  std::vector<Animation> m_animations;
  std::vector<Transitions> m_transitions;
  std::vector<TransitionCase> m_transitionCases;
  std::vector<Box> m_boxes;
  std::unordered_map<core::StaticMeshId, StaticMesh> m_staticMeshes;
  std::vector<Mesh> m_meshes;
  std::map<core::TypeId, std::unique_ptr<SkeletalModelType>> m_animatedModels;
  std::vector<Sprite> m_sprites;
  std::map<core::TypeId, std::unique_ptr<SpriteSequence>> m_spriteSequences;
  std::vector<AtlasTile> m_atlasTiles;
  std::vector<Room> m_rooms;
  std::vector<CinematicFrame> m_cinematicFrames;
  std::vector<CameraSink> m_cameraSinks;
  std::vector<StaticSoundEffect> m_staticSoundEffects;

  ControllerLayouts m_controllerLayouts;

  void initTextureDependentDataFromLevel(const loader::file::level::Level& level);
  void initFromLevel(loader::file::level::Level& level);
  void connectSectors();
  void updateStaticSoundEffects();
};
} // namespace engine::world
