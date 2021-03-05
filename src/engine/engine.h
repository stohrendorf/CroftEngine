#pragma once

#include "engineconfig.h"
#include "inventory.h"

#include <boost/assert.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <pybind11/embed.h>

namespace loader::trx
{
class Glidos;
}

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

class I18nProvider;
class Particle;
class Player;
class Presenter;
class Throttler;
class World;

enum class RunResult
{
  NextLevel,
  TitleLevel,
  LaraHomeLevel,
  ExitApp,
  RequestLoad,
};

struct SavegameMeta
{
  std::string filename;
  std::string title;

  void serialize(const serialization::Serializer<SavegameMeta>& ser);
};

inline std::string makeSavegameFilename(size_t n)
{
  return "save_" + std::to_string(n) + ".yaml";
}

class Engine
{
private:
  const std::filesystem::path m_rootPath;
  EngineConfig m_engineConfig;
  std::shared_ptr<Presenter> m_presenter;

  std::shared_ptr<pybind11::scoped_interpreter> m_scriptEngine;

  std::string m_language;
  std::unique_ptr<I18nProvider> m_i18n;

  std::unique_ptr<loader::trx::Glidos> m_glidos;
  [[nodiscard]] std::unique_ptr<loader::trx::Glidos> loadGlidosPack() const;

  void makeScreenshot();

public:
  explicit Engine(const std::filesystem::path& rootPath,
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

  std::pair<RunResult, std::optional<size_t>> run(World& world, bool isCutscene, bool allowSave);
  std::pair<RunResult, std::optional<size_t>> runTitleMenu(World& world);

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

  std::pair<RunResult, std::optional<size_t>> runLevelSequenceItem(script::LevelSequenceItem& item,
                                                                   const std::shared_ptr<Player>& player);
  std::pair<RunResult, std::optional<size_t>> runLevelSequenceItemFromSave(script::LevelSequenceItem& item,
                                                                           const std::optional<size_t>& slot,
                                                                           const std::shared_ptr<Player>& player);

  [[nodiscard]] const auto& getGlidos() const noexcept
  {
    return m_glidos;
  }

  SavegameMeta getSavegameMeta(const std::filesystem::path& filename) const;
  SavegameMeta getSavegameMeta(size_t slot) const
  {
    return getSavegameMeta(makeSavegameFilename(slot));
  }

  auto& getEngineConfig()
  {
    return m_engineConfig;
  }

  const auto& getEngineConfig() const
  {
    return m_engineConfig;
  }

  const auto& i18n() const
  {
    return *m_i18n;
  }
};
} // namespace engine
