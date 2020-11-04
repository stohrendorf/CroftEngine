#pragma once

#include "cameracontroller.h"
#include "engine/objects/modelobject.h"
#include "floordata/floordata.h"
#include "inventory.h"
#include "items_tr1.h"
#include "loader/file/animationid.h"
#include "loader/file/item.h"
#include "objectmanager.h"

#include <filesystem>
#include <gl/pixel.h>
#include <gl/texture2darray.h>
#include <memory>
#include <pybind11/embed.h>

namespace loader::file
{
namespace level
{
class Level;
}

struct Room;
struct Sector;
struct Mesh;
struct SkeletalModelType;
struct Box;
struct StaticMesh;
struct SpriteSequence;
struct AnimFrame;
struct Animation;
struct CinematicFrame;
class RenderMeshData;
} // namespace loader::file

namespace engine
{
namespace objects
{
class Object;

class PickupObject;
} // namespace objects

class Particle;
class Presenter;
class Throttler;

class Engine
{
private:
  const std::filesystem::path m_rootPath;
  std::shared_ptr<Presenter> m_presenter;

  std::shared_ptr<loader::file::level::Level> m_level;
  std::unique_ptr<CameraController> m_cameraController = nullptr;

  core::Frame m_effectTimer = 0_frame;
  std::optional<size_t> m_activeEffect{};

  std::shared_ptr<pybind11::scoped_interpreter> m_scriptEngine;

  bool m_roomsAreSwapped = false;
  std::vector<size_t> m_roomOrder;

  // list of meshes and models, resolved through m_meshIndices
  std::vector<gsl::not_null<const loader::file::Mesh*>> m_meshesDirect;

  ObjectManager m_objectManager;

  pybind11::dict levelInfo;
  std::string language;
  bool m_levelFinished = false;

  Inventory m_inventory{};

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

  std::string loadLevel();

public:
  explicit Engine(const std::filesystem::path& rootPath,
                  bool fullscreen = false,
                  const glm::ivec2& resolution = {1280, 800});

  ~Engine();

  [[nodiscard]] const auto& getLevel() const
  {
    BOOST_ASSERT(m_level != nullptr);
    return *m_level;
  }

  [[nodiscard]] const auto& getPresenter() const
  {
    BOOST_ASSERT(m_presenter != nullptr);
    return *m_presenter;
  }

  [[nodiscard]] auto& getPresenter()
  {
    BOOST_ASSERT(m_presenter != nullptr);
    return *m_presenter;
  }

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

  auto& getScriptEngine()
  {
    return *m_scriptEngine;
  }

  [[nodiscard]] const auto& getScriptEngine() const
  {
    return *m_scriptEngine;
  }

  auto& getInventory()
  {
    return m_inventory;
  }

  [[nodiscard]] const auto& getInventory() const
  {
    return m_inventory;
  }

  [[nodiscard]] bool hasLevel() const
  {
    return m_level != nullptr;
  }

  void finishLevel()
  {
    m_levelFinished = true;
  }

  void run();

  void loadSceneData(const std::string& animatedTextureId);

  [[nodiscard]] const std::unique_ptr<loader::file::SkeletalModelType>&
    findAnimatedModelForType(core::TypeId type) const;

  template<typename T>
  std::shared_ptr<T> createObject(const core::TypeId type,
                                  const gsl::not_null<const loader::file::Room*>& room,
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

    auto object = std::make_shared<T>(this, room, item, model.get());

    m_objectManager.registerDynamicObject(object);
    addChild(room->node, object->getNode());

    return object;
  }

  std::shared_ptr<objects::PickupObject>
    createPickup(core::TypeId type, const gsl::not_null<const loader::file::Room*>& room, const core::TRVec& position);

  static std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                                      const core::TRVec& position);

  void useAlternativeLaraAppearance(bool withHead = false);

  [[nodiscard]] gsl::not_null<std::shared_ptr<loader::file::RenderMeshData>> getRenderMesh(size_t idx) const;

  [[nodiscard]] const std::vector<loader::file::Mesh>& getMeshes() const;

  void turn180Effect(objects::Object& object);

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

  void unholsterRightGunEffect(objects::ModelObject& object);

  void chainBlockEffect();

  void flickerEffect();

  void swapAllRooms();

  void setGlobalEffect(size_t fx)
  {
    m_activeEffect = fx;
    m_effectTimer = 0_frame;
  }

  void doGlobalEffect();

  void runEffect(const size_t id, objects::Object* object)
  {
    switch(id)
    {
    case 0: Expects(object != nullptr); return turn180Effect(*object);
    case 1: Expects(object != nullptr); return dinoStompEffect(*object);
    case 2: return laraNormalEffect();
    case 3: Expects(object != nullptr); return laraBubblesEffect(*object);
    case 4: return finishLevelEffect();
    case 5: return earthquakeEffect();
    case 6: return floodEffect();
    case 7: return chandelierEffect();
    case 8: return raisingBlockEffect();
    case 9: return stairsToSlopeEffect();
    case 10: return sandEffect();
    case 11: return explosionEffect();
    case 12: return laraHandsFreeEffect();
    case 13: return flipMapEffect();
    case 14:
      Expects(object != nullptr);
      if(const auto m = dynamic_cast<objects::ModelObject*>(object))
        return unholsterRightGunEffect(*m);
      break;
    case 15: return chainBlockEffect();
    case 16: return flickerEffect();
    default: BOOST_LOG_TRIVIAL(warning) << "Unhandled effect: " << id;
    }
  }

  void swapWithAlternate(loader::file::Room& orig, loader::file::Room& alternate);

  void serialize(const serialization::Serializer& ser);

  std::array<floordata::ActivationState, 10> mapFlipActivationStates;

  objects::Object* m_pierre = nullptr;

  [[nodiscard]] const std::vector<loader::file::Box>& getBoxes() const;

  [[nodiscard]] const std::vector<loader::file::Room>& getRooms() const;
  std::vector<loader::file::Room>& getRooms();

  [[nodiscard]] const engine::floordata::FloorData& getFloorData() const;

  [[nodiscard]] const loader::file::StaticMesh* findStaticMeshById(core::StaticMeshId meshId) const;

  [[nodiscard]] const std::unique_ptr<loader::file::SpriteSequence>& findSpriteSequenceForType(core::TypeId type) const;

  bool isValid(const loader::file::AnimFrame* frame) const;

  [[nodiscard]] const loader::file::Animation& getAnimation(loader::file::AnimationId id) const;

  [[nodiscard]] const std::vector<loader::file::Animation>& getAnimations() const;

  [[nodiscard]] const std::vector<loader::file::CinematicFrame>& getCinematicFrames() const;

  [[nodiscard]] const std::vector<loader::file::Camera>& getCameras() const;

  [[nodiscard]] const std::vector<int16_t>& getAnimCommands() const;

  [[nodiscard]] const std::vector<uint16_t>& getOverlaps() const;

  void update(bool godMode);

  [[nodiscard]] const std::vector<int16_t>& getPoseFrames() const;

  void handleCommandSequence(const floordata::FloorDataValue* floorData, bool fromHeavy);

  core::TypeId find(const loader::file::SkeletalModelType* model) const;
  core::TypeId find(const loader::file::Sprite* sprite) const;

  [[nodiscard]] const pybind11::dict& getLevelInfo() const
  {
    return levelInfo;
  }

  [[nodiscard]] const std::string& getLanguage() const
  {
    return language;
  }

  [[nodiscard]] const loader::file::Palette& getPalette() const;

  void gameLoop(Throttler& throttler, const std::string& levelName, bool godMode);
  void cinematicLoop(Throttler& throttler);
};
} // namespace engine
