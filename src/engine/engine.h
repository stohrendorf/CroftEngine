#pragma once

#include "audioengine.h"
#include "cameracontroller.h"
#include "floordata/floordata.h"
#include "inventory.h"
#include "items_tr1.h"
#include "loader/file/animationid.h"
#include "loader/file/item.h"
#include "objectmanager.h"
#include "render/scene/materialmanager.h"
#include "render/scene/screenoverlay.h"

#include <engine/objects/modelobject.h>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/pixel.h>
#include <gl/texture2darray.h>
#include <memory>

namespace hid
{
class InputHandler;
}

namespace loader
{
namespace file
{
namespace level
{
class Level;
}

struct TextureKey;
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
} // namespace file
} // namespace loader

namespace gl
{
class Font;
}

namespace render
{
class RenderPipeline;
} // namespace render

namespace engine
{
namespace objects
{
class Object;

class PickupObject;
} // namespace objects

class Particle;
class Engine
{
private:
  const std::filesystem::path m_rootPath;

  std::shared_ptr<loader::file::level::Level> m_level;
  std::unique_ptr<CameraController> m_cameraController = nullptr;

  std::unique_ptr<AudioEngine> m_audioEngine;

  core::Frame m_effectTimer = 0_frame;
  std::optional<size_t> m_activeEffect{};

  int m_uvAnimTime{0};

  sol::state m_scriptEngine;

  std::shared_ptr<render::TextureAnimator> m_textureAnimator;

  std::unique_ptr<hid::InputHandler> m_inputHandler;

  bool m_roomsAreSwapped = false;

  // list of meshes and models, resolved through m_meshIndices
  std::vector<gsl::not_null<const loader::file::Mesh*>> m_meshesDirect;

  ObjectManager m_objectManager;

  std::shared_ptr<render::RenderPipeline> m_renderPipeline;
  std::shared_ptr<render::scene::ScreenOverlay> screenOverlay;
  std::unique_ptr<gl::Window> m_window;
  std::shared_ptr<render::scene::Renderer> m_renderer;
  sol::table levelInfo;

  const gl::CImgWrapper splashImage;
  gl::CImgWrapper splashImageScaled;
  std::shared_ptr<gl::Font> abibasFont;

  bool m_levelFinished = false;

  Inventory m_inventory{};

  std::shared_ptr<gl::Texture2DArray<gl::SRGBA8>> m_allTextures;

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
  core::Health m_drawnHealth = core::LaraHealth;
  core::Frame m_healthBarTimeout = -40_frame;

  std::bitset<16> m_secretsFoundBitmask = 0;

  std::shared_ptr<render::scene::CSM> m_csm{};
  std::unique_ptr<render::scene::MaterialManager> m_materialManager{};
  std::shared_ptr<render::scene::ShaderManager> m_shaderManager{};

public:
  explicit Engine(const std::filesystem::path& rootPath,
                  bool fullscreen = false,
                  const glm::ivec2& resolution = {1280, 800});

  ~Engine();

  const hid::InputHandler& getInputHandler() const
  {
    return *m_inputHandler;
  }

  bool roomsAreSwapped() const
  {
    return m_roomsAreSwapped;
  }

  CameraController& getCameraController()
  {
    return *m_cameraController;
  }

  const CameraController& getCameraController() const
  {
    return *m_cameraController;
  }

  ObjectManager& getObjectManager()
  {
    return m_objectManager;
  }

  const ObjectManager& getObjectManager() const
  {
    return m_objectManager;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  auto& getSoundEngine()
  {
    return m_audioEngine->getSoundEngine();
  }

  auto& getScriptEngine()
  {
    return m_scriptEngine;
  }

  const auto& getScriptEngine() const
  {
    return m_scriptEngine;
  }

  auto& getAudioEngine()
  {
    return *m_audioEngine;
  }

  const auto& getAudioEngine() const
  {
    return *m_audioEngine;
  }

  auto& getInventory()
  {
    return m_inventory;
  }

  const auto& getInventory() const
  {
    return m_inventory;
  }

  bool hasLevel() const
  {
    return m_level != nullptr;
  }

  void finishLevel()
  {
    m_levelFinished = true;
  }

  const auto& getMaterialManager()
  {
    return m_materialManager;
  }

  const auto& getMaterialManager() const
  {
    return m_materialManager;
  }

  void run();

  void loadSceneData();

  const std::unique_ptr<loader::file::SkeletalModelType>& findAnimatedModelForType(core::TypeId type) const;

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

  std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                               const core::TRVec& position) const;

  void drawBars(const gsl::not_null<std::shared_ptr<gl::Image<gl::SRGBA8>>>& image);

  void useAlternativeLaraAppearance(bool withHead = false);

  gsl::not_null<std::shared_ptr<loader::file::RenderMeshData>> getRenderMesh(const size_t idx) const;

  const std::vector<loader::file::Mesh>& getMeshes() const;

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
    case 15: return chainBlockEffect();
    case 16: return flickerEffect();
    default: BOOST_LOG_TRIVIAL(warning) << "Unhandled effect: " << id;
    }
  }

  void swapWithAlternate(loader::file::Room& orig, loader::file::Room& alternate);

  void animateUV();

  void serialize(const serialization::Serializer& ser);

  std::array<floordata::ActivationState, 10> mapFlipActivationStates;

  objects::Object* m_pierre = nullptr;

  const std::vector<loader::file::Box>& getBoxes() const;

  const std::vector<loader::file::Room>& getRooms() const;
  std::vector<loader::file::Room>& getRooms();

  const engine::floordata::FloorData& getFloorData() const;

  const loader::file::StaticMesh* findStaticMeshById(core::StaticMeshId meshId) const;

  const std::unique_ptr<loader::file::SpriteSequence>& findSpriteSequenceForType(core::TypeId type) const;

  bool isValid(const loader::file::AnimFrame* frame) const;

  const loader::file::Animation& getAnimation(loader::file::AnimationId id) const;

  const std::vector<loader::file::Animation>& getAnimations() const;

  const std::vector<loader::file::CinematicFrame>& getCinematicFrames() const;

  const std::vector<loader::file::Camera>& getCameras() const;

  const std::vector<int16_t>& getAnimCommands() const;

  const std::vector<uint16_t>& getOverlaps() const;

  void update(bool godMode);

  static void drawText(gl::Image<gl::SRGBA8>& img,
                       const gsl::not_null<std::shared_ptr<gl::Font>>& font,
                       int x,
                       const int y,
                       const std::string& txt,
                       const gl::SRGBA8& col = {255, 255, 255, 255});

  void drawDebugInfo(gl::Image<gl::SRGBA8>& img, const gsl::not_null<std::shared_ptr<gl::Font>>& font, float fps);

  void scaleSplashImage();

  void drawLoadingScreen(const std::string& state);

  const std::vector<int16_t>& getPoseFrames() const;

  void handleCommandSequence(const floordata::FloorDataValue* floorData, bool fromHeavy);

  core::TypeId find(const loader::file::SkeletalModelType* model) const;
  core::TypeId find(const loader::file::Sprite* sprite) const;
};
} // namespace engine
