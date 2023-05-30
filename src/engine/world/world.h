#pragma once

#include "animation.h"
#include "atlastile.h"
#include "audio/emitter.h"
#include "box.h"
#include "camerasink.h"
#include "cinematicframe.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/controllerbuttons.h"
#include "engine/floordata/types.h"
#include "engine/items_tr1.h"
#include "engine/objectmanager.h"
#include "engine/objects/object.h"
#include "loader/file/item.h"
#include "mesh.h"
#include "qs/qs.h"
#include "room.h"
#include "serialization/serialization_fwd.h"
#include "sprite.h"
#include "staticmesh.h"
#include "staticsoundeffect.h"
#include "transition.h"
#include "ui/pickupwidget.h"
#include "worldgeometry.h"

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace audio
{
class SoundEngine;
class Voice;
} // namespace audio

namespace engine::floordata
{
class ActivationState;
}

namespace ui
{
class Ui;
}

namespace loader::file
{
enum class AnimationId : uint16_t;
struct AnimFrame;
} // namespace loader::file

namespace loader::file::level
{
class Level;
}

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
struct Location;
} // namespace engine

namespace engine::world
{
class RenderMeshData;
struct SkeletalModelType;

class World final
{
public:
  explicit World(const gsl::not_null<Engine*>& engine,
                 std::unique_ptr<loader::file::level::Level>&& level,
                 std::string title,
                 const std::optional<TR1TrackId>& ambient,
                 bool useAlternativeLara,
                 std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
                 std::shared_ptr<Player> player,
                 std::shared_ptr<Player> levelStartPlayer,
                 bool fromSave);

  ~World();

  [[nodiscard]] bool roomsAreSwapped() const noexcept
  {
    return m_roomsAreSwapped;
  }

  CameraController& getCameraController() noexcept
  {
    return *m_cameraController;
  }

  [[nodiscard]] const CameraController& getCameraController() const noexcept
  {
    return *m_cameraController;
  }

  ObjectManager& getObjectManager() noexcept
  {
    return m_objectManager;
  }

  [[nodiscard]] const ObjectManager& getObjectManager() const noexcept
  {
    return m_objectManager;
  }

  void finishLevel() noexcept
  {
    m_levelFinished = true;
  }

  [[nodiscard]] bool levelFinished() const noexcept
  {
    return m_levelFinished;
  }

  void setGlobalEffect(size_t fx) noexcept
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
    const auto& model = m_worldGeometry.findAnimatedModelForType(type);
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
      = gsl::make_shared<T>(objects::makeObjectName(type.get_as<TR1ItemId>(), m_objectManager.getDynamicObjectCount()),
                            gsl::not_null{this},
                            room,
                            item,
                            gsl::not_null{model.get()});

    m_objectManager.registerDynamicObject(object);
    addChild(gsl::not_null{room->node}, gsl::not_null{object->getNode()});

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
  void swapWithAlternate(Room& orig, Room& alternate);
  [[nodiscard]] const std::vector<Box>& getBoxes() const noexcept;
  [[nodiscard]] const std::vector<Room>& getRooms() const noexcept;
  std::vector<Room>& getRooms() noexcept;
  [[nodiscard]] const std::vector<CinematicFrame>& getCinematicFrames() const noexcept;
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
  gslu::nn_shared<objects::PickupObject>
    createPickup(const core::TypeId& type, const gsl::not_null<const Room*>& room, const core::TRVec& position);
  void useAlternativeLaraAppearance(bool withHead = false);
  void runEffect(size_t id, objects::Object* object);
  void turn180Effect(objects::Object& object) noexcept;
  void drawRightWeaponEffect(const objects::ModelObject& object);
  [[nodiscard]] const std::array<gl::SRGBA8, 256>& getPalette() const noexcept;
  void handleCommandSequence(const floordata::FloorDataValue* floorData, bool fromHeavy);
  void serialize(const serialization::Serializer<World>& ser) const;
  void deserialize(const serialization::Deserializer<World>& ser);
  void gameLoop(bool godMode, float blackAlpha, ui::Ui& ui);
  bool cinematicLoop();
  void load(const std::optional<size_t>& slot);
  void save(const std::optional<size_t>& slot);
  void save(const std::filesystem::path& filename);
  [[nodiscard]] std::tuple<std::optional<SavegameInfo>, std::map<size_t, SavegameInfo>> getSavedGames() const;
  [[nodiscard]] bool hasSavedGames() const;

  [[nodiscard]] const Presenter& getPresenter() const;
  [[nodiscard]] Presenter& getPresenter();

  [[nodiscard]] auto getPierre() const noexcept
  {
    return m_pierre;
  }

  void setPierre(objects::Object* pierre) noexcept
  {
    m_pierre = m_objectManager.find(pierre);
  }

  [[nodiscard]] const Engine& getEngine() const
  {
    return *m_engine;
  }

  Engine& getEngine()
  {
    return *m_engine;
  }

  [[nodiscard]] const AudioEngine& getAudioEngine() const noexcept
  {
    return *m_audioEngine;
  }

  AudioEngine& getAudioEngine() noexcept
  {
    return *m_audioEngine;
  }

  [[nodiscard]] const std::string& getTitle() const noexcept
  {
    return m_title;
  }

  [[nodiscard]] auto getTotalSecrets() const noexcept
  {
    return m_totalSecrets;
  }

  void addPickupWidget(Sprite sprite, size_t count)
  {
    static auto constexpr widgetLifetime = 75_frame;
    auto isSameVirginItem = [&sprite](const ui::PickupWidget& item) -> bool
    {
      return item.getSprite() == sprite && item.getDuration() == widgetLifetime;
    };
    if(auto it = std::find_if(m_pickupWidgets.begin(), m_pickupWidgets.end(), isSameVirginItem);
       it != m_pickupWidgets.end())
    {
      it->setCount(count); // at this point the picked up ammo is already in the player's inventory
      return;
    }
    m_pickupWidgets.emplace_back(widgetLifetime, std::move(sprite), count);
  }

  [[nodiscard]] std::optional<std::string> getItemTitle(TR1ItemId id) const;

  auto& getPlayer()
  {
    gsl_Expects(m_player != nullptr);
    return *m_player;
  }

  [[nodiscard]] const auto& getPlayer() const
  {
    gsl_Expects(m_player != nullptr);
    return *m_player;
  }

  [[nodiscard]] const auto& getPlayerPtr() const noexcept
  {
    return m_player;
  }

  [[nodiscard]] const auto& getWorldGeometry() const noexcept
  {
    return m_worldGeometry;
  }

  [[nodiscard]] const auto& getFloorData() const noexcept
  {
    return m_floorData;
  }

  [[nodiscard]] const auto& getCameraSinks() const noexcept
  {
    return m_cameraSinks;
  }

  [[nodiscard]] const auto& getControllerLayouts() const noexcept
  {
    return m_controllerLayouts;
  }

  [[nodiscard]] auto& getMapFlipActivationStates() noexcept
  {
    return m_mapFlipActivationStates;
  }

  [[nodiscard]] const auto& getLevelFilename() const noexcept
  {
    return m_levelFilename;
  }

  [[nodiscard]] const auto& getGhostFrame() const noexcept
  {
    return m_ghostFrame;
  }

  void nextGhostFrame() noexcept
  {
    m_ghostFrame += 1_frame;
  }

private:
  void drawPickupWidgets(ui::Ui& ui);

  gsl::not_null<Engine*> m_engine;
  std::filesystem::path m_levelFilename;

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
  std::shared_ptr<objects::Object> m_pierre = nullptr;
  std::string m_title{};
  size_t m_totalSecrets = 0;
  std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> m_itemTitles{};
  std::shared_ptr<gl::Texture2DArray<gl::PremultipliedSRGBA8>> m_allTextures;
  core::Frame m_uvAnimTime = 0_frame;

  std::vector<ui::PickupWidget> m_pickupWidgets{};
  std::shared_ptr<Player> m_player;
  std::shared_ptr<Player> m_levelStartPlayer;

  engine::floordata::FloorData m_floorData;
  std::array<gl::SRGBA8, 256> m_palette;
  std::vector<uint8_t> m_samplesData;
  WorldGeometry m_worldGeometry{};

  std::vector<Box> m_boxes;
  std::vector<Room> m_rooms;
  std::vector<CinematicFrame> m_cinematicFrames;
  std::vector<CameraSink> m_cameraSinks;
  std::vector<StaticSoundEffect> m_staticSoundEffects;

  ControllerLayouts m_controllerLayouts;

  core::Frame m_ghostFrame = 0_frame;

  static constexpr auto DeathStrengthFadeDuration = 1_sec * core::FrameRate;
  static constexpr auto DeathStrengthFadeDeltaPerFrame = 1_frame / DeathStrengthFadeDuration.cast<float>();
  float m_currentDeathStrength = 0;

  void initFromLevel(loader::file::level::Level& level, bool fromSave);
  void connectSectors();
  void updateStaticSoundEffects();

  void initBoxes(const loader::file::level::Level& level);
  void initRooms(const loader::file::level::Level& level);
  void initCinematicFrames(const loader::file::level::Level& level);
  void initCameras(const loader::file::level::Level& level);
  void countSecrets();
  void initCameraController();
  void initStaticSoundEffects(const loader::file::level::Level& level);
};
} // namespace engine::world
