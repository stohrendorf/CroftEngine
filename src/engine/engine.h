#pragma once

#include "engineconfig.h"
#include "inventory.h"
#include "script/scriptengine.h"

#include <boost/assert.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <set>

namespace loader::trx
{
class Glidos;
}

namespace loader::file::level
{
class Level;
} // namespace loader::file::level

namespace engine::world
{
class World;
class RenderMeshData;
} // namespace engine::world

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
class Player;
class Presenter;
class Throttler;

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

struct SavegameInfo
{
  SavegameMeta meta{};
  std::filesystem::file_time_type saveTime{};
};

inline std::string makeSavegameFilename(size_t n)
{
  return "save_" + std::to_string(n) + ".yaml";
}

class Engine
{
private:
  const std::filesystem::path m_rootPath;
  script::ScriptEngine m_scriptEngine;
  std::unique_ptr<EngineConfig> m_engineConfig;
  std::shared_ptr<Presenter> m_presenter;
  std::set<gsl::not_null<world::World*>> m_worlds;

  std::string m_locale;

  std::unique_ptr<loader::trx::Glidos> m_glidos;
  [[nodiscard]] std::unique_ptr<loader::trx::Glidos> loadGlidosPack() const;

  void makeScreenshot();

public:
  explicit Engine(const std::filesystem::path& rootPath, const glm::ivec2& resolution = {1280, 800});

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

  std::pair<RunResult, std::optional<size_t>> run(world::World& world, bool isCutscene, bool allowSave);
  std::pair<RunResult, std::optional<size_t>> runTitleMenu(world::World& world);

  [[nodiscard]] const std::string& getLocale() const
  {
    return m_locale;
  }

  [[nodiscard]] std::string getLocaleWithoutEncoding() const
  {
    if(auto idx = m_locale.find('.'); idx != std::string::npos)
      return m_locale.substr(0, idx);
    return m_locale;
  }

  [[nodiscard]] std::filesystem::path getSavegameRootPath() const;
  [[nodiscard]] std::filesystem::path getSavegamePath(const std::optional<size_t>& slot) const;

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

  [[nodiscard]] std::optional<SavegameMeta> getSavegameMeta(const std::filesystem::path& filename) const;
  [[nodiscard]] std::optional<SavegameMeta> getSavegameMeta(const std::optional<size_t>& slot) const;

  auto& getEngineConfig()
  {
    return m_engineConfig;
  }

  [[nodiscard]] const auto& getEngineConfig() const
  {
    return m_engineConfig;
  }

  void registerWorld(world::World* world)
  {
    m_worlds.emplace(world);
  }

  void unregisterWorld(world::World* world)
  {
    m_worlds.erase(world);
  }

  void applySettings();

  const auto& getScriptEngine() const
  {
    return m_scriptEngine;
  }
};
} // namespace engine
