#pragma once

#include "audioengine.h"
#include "cameracontroller.h"
#include "floordata/floordata.h"
#include "inventory.h"
#include "items_tr1.h"
#include "loader/file/animationid.h"
#include "loader/file/item.h"
#include "render/scene/screenoverlay.h"
#include "util/cimgwrapper.h"

#include <boost/filesystem/path.hpp>
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
} // namespace file
} // namespace loader

namespace render
{
namespace gl
{
class Font;
}

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
using ObjectId = uint16_t;

class Engine
{
private:
  std::shared_ptr<loader::file::level::Level> m_level;
  std::unique_ptr<CameraController> m_cameraController = nullptr;

  std::unique_ptr<AudioEngine> m_audioEngine;

  core::Frame m_effectTimer = 0_frame;
  std::optional<size_t> m_activeEffect{};

  uint16_t m_objectCounter = 0x8000;
  std::map<ObjectId, gsl::not_null<std::shared_ptr<objects::Object>>> m_objects;

  std::set<gsl::not_null<std::shared_ptr<objects::Object>>> m_dynamicObjects;

  std::set<objects::Object*> m_scheduledDeletions;

  std::vector<gsl::not_null<std::shared_ptr<render::scene::Model>>> m_models;

  int m_uvAnimTime{0};

  std::shared_ptr<render::scene::ShaderProgram> m_lightningShader;

  sol::state m_scriptEngine;

  std::shared_ptr<objects::LaraObject> m_lara = nullptr;

  std::shared_ptr<render::TextureAnimator> m_textureAnimator;

  std::unique_ptr<hid::InputHandler> m_inputHandler;

  bool m_roomsAreSwapped = false;

  std::vector<gsl::not_null<std::shared_ptr<Particle>>> m_particles;

  // list of meshes and models, resolved through m_meshIndices
  std::vector<gsl::not_null<std::shared_ptr<render::scene::Model>>> m_modelsDirect;
  std::vector<gsl::not_null<const loader::file::Mesh*>> m_meshesDirect;

  std::shared_ptr<render::scene::Material> m_spriteMaterial{nullptr};
  std::shared_ptr<render::scene::Material> m_portalMaterial{nullptr};

  std::shared_ptr<render::RenderPipeline> m_renderPipeline;
  std::shared_ptr<render::scene::ScreenOverlay> screenOverlay;
  std::unique_ptr<render::scene::Renderer> m_renderer;
  std::unique_ptr<render::scene::Window> m_window;
  sol::table levelInfo;

  const util::CImgWrapper splashImage;
  util::CImgWrapper splashImageScaled;
  std::shared_ptr<render::gl::Font> abibasFont;

  bool m_levelFinished = false;

  Inventory m_inventory;

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

public:
  explicit Engine(bool fullscreen = false, const render::scene::Dimension2<int>& resolution = {1280, 800});

  ~Engine();

  const hid::InputHandler& getInputHandler() const
  {
    return *m_inputHandler;
  }

  bool roomsAreSwapped() const
  {
    return m_roomsAreSwapped;
  }

  objects::LaraObject& getLara()
  {
    return *m_lara;
  }

  const objects::LaraObject& getLara() const
  {
    return *m_lara;
  }

  auto& getParticles()
  {
    return m_particles;
  }

  auto& getObjects()
  {
    return m_objects;
  }

  const auto& getObjects() const
  {
    return m_objects;
  }

  const auto& getDynamicObjects() const
  {
    return m_dynamicObjects;
  }

  CameraController& getCameraController()
  {
    return *m_cameraController;
  }

  const CameraController& getCameraController() const
  {
    return *m_cameraController;
  }

  const auto& getSpriteMaterial() const
  {
    return m_spriteMaterial;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  auto& getSoundEngine()
  {
    return m_audioEngine->m_soundEngine;
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

  void run();

  std::map<loader::file::TextureKey, gsl::not_null<std::shared_ptr<render::scene::Material>>>
    createMaterials(const gsl::not_null<std::shared_ptr<render::scene::ShaderProgram>>& shader) const;

  std::shared_ptr<objects::LaraObject> createObjects();

  void loadSceneData(bool linearTextureInterpolation);

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
    item.darkness = 0;
    item.activationState = activationState;

    auto object = std::make_shared<T>(this, room, item, model.get());

    m_dynamicObjects.emplace(object);
    addChild(room->node, object->getNode());

    return object;
  }

  std::shared_ptr<objects::PickupObject>
    createPickup(core::TypeId type, const gsl::not_null<const loader::file::Room*>& room, const core::TRVec& position);

  std::tuple<int8_t, int8_t> getFloorSlantInfo(gsl::not_null<const loader::file::Sector*> sector,
                                               const core::TRVec& position) const;

  std::shared_ptr<objects::Object> getObject(uint16_t id) const;

  void drawBars(const gsl::not_null<std::shared_ptr<render::gl::Image<render::gl::SRGBA8>>>& image);

  void useAlternativeLaraAppearance(bool withHead = false);

  const gsl::not_null<std::shared_ptr<render::scene::Model>>& getModel(const size_t idx) const
  {
    return m_models.at(idx);
  }

  void scheduleDeletion(objects::Object* object)
  {
    m_scheduledDeletions.insert(object);
  }

  void applyScheduledDeletions()
  {
    if(m_scheduledDeletions.empty())
      return;

    for(const auto& del : m_scheduledDeletions)
    {
      auto it = std::find_if(m_dynamicObjects.begin(),
                             m_dynamicObjects.end(),
                             [del](const std::shared_ptr<objects::Object>& i) { return i.get() == del; });
      if(it != m_dynamicObjects.end())
      {
        m_dynamicObjects.erase(it);
        continue;
      }

      auto it2 = std::find_if(m_objects.begin(),
                              m_objects.end(),
                              [del](const std::pair<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& i) {
                                return i.second.get().get() == del;
                              });
      if(it2 != m_objects.end())
      {
        m_objects.erase(it2);
        continue;
      }
    }

    m_scheduledDeletions.clear();
  }

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

  void unholsterRightGunEffect(objects::Object& object);

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
    case 14: Expects(object != nullptr); return unholsterRightGunEffect(*object);
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

  static void drawText(const gsl::not_null<std::shared_ptr<render::gl::Font>>& font,
                       int x,
                       const int y,
                       const std::string& txt,
                       const render::gl::SRGBA8& col = {255, 255, 255, 255});

  void drawDebugInfo(const gsl::not_null<std::shared_ptr<render::gl::Font>>& font, float fps);

  void scaleSplashImage();

  void drawLoadingScreen(const std::string& state);

  const std::vector<int16_t>& getPoseFrames() const;

  void registerObject(const gsl::not_null<std::shared_ptr<objects::Object>>& object)
  {
    if(m_objectCounter == std::numeric_limits<uint16_t>::max())
      BOOST_THROW_EXCEPTION(std::runtime_error("Artificial object counter exceeded"));

    m_objects.emplace(m_objectCounter++, object);
  }

  std::shared_ptr<objects::Object> find(const objects::Object* object) const
  {
    if(object == nullptr)
      return nullptr;

    auto it = std::find_if(m_objects.begin(),
                           m_objects.end(),
                           [object](const std::pair<uint16_t, gsl::not_null<std::shared_ptr<objects::Object>>>& x) {
                             return x.second.get().get() == object;
                           });

    if(it == m_objects.end())
      return nullptr;

    return it->second;
  }

  void handleCommandSequence(const floordata::FloorDataValue* floorData, bool fromHeavy);

  core::TypeId find(const loader::file::SkeletalModelType* model) const;
  core::TypeId find(const loader::file::Sprite* sprite) const;

  const auto& getLightningShader() const
  {
    return m_lightningShader;
  }

  std::optional<size_t> indexOfModel(const std::shared_ptr<render::scene::Renderable>& m) const
  {
    if(m == nullptr)
      return std::nullopt;

    for(size_t i = 0; i < m_models.size(); ++i)
      if(m_models[i].get() == m)
        return i;

    return std::nullopt;
  }
};
} // namespace engine
