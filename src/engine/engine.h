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
class World;

enum class RunResult
{
  NextLevel,
  TitleLevel,
  LaraHomeLevel,
  ExitApp,
};

class Engine
{
private:
  const std::filesystem::path m_rootPath;
  std::shared_ptr<Presenter> m_presenter;

  std::shared_ptr<pybind11::scoped_interpreter> m_scriptEngine;

  std::string language;

  std::unique_ptr<World> m_world;

public:
  explicit Engine(const std::filesystem::path& rootPath,
                  bool fullscreen = false,
                  const glm::ivec2& resolution = {1280, 800});

  ~Engine();

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

  auto& getScriptEngine()
  {
    return *m_scriptEngine;
  }

  [[nodiscard]] const auto& getScriptEngine() const
  {
    return *m_scriptEngine;
  }

  RunResult run();
  RunResult runTitleMenu();

  [[nodiscard]] const std::string& getLanguage() const
  {
    return language;
  }

  [[nodiscard]] std::filesystem::path getSavegamePath() const
  {
    auto p = m_rootPath / "saves";
    if(!std::filesystem::is_directory(p))
      std::filesystem::create_directory(p);
    return p;
  }

  const std::filesystem::path& getRootPath() const
  {
    return m_rootPath;
  }

  void loadWorld(size_t levelIndex);
};
} // namespace engine
