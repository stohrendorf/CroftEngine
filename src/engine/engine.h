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

namespace script
{
class LevelSequenceItem;
}

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

  std::string m_language;

  std::unique_ptr<loader::trx::Glidos> m_glidos;
  [[nodiscard]] std::unique_ptr<loader::trx::Glidos> loadGlidosPack() const;

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

  RunResult run(World& world, bool isCutscene);
  RunResult runTitleMenu(World& world);

  [[nodiscard]] const std::string& getLanguage() const
  {
    return m_language;
  }

  [[nodiscard]] std::filesystem::path getSavegamePath() const
  {
    auto p = m_rootPath / "saves";
    if(!std::filesystem::is_directory(p))
      std::filesystem::create_directory(p);
    return p;
  }

  [[nodiscard]] const std::filesystem::path& getRootPath() const
  {
    return m_rootPath;
  }

  RunResult runLevelSequenceItem(script::LevelSequenceItem& item);

  [[nodiscard]] const auto& getGlidos() const noexcept
  {
    return m_glidos;
  }
};
} // namespace engine
