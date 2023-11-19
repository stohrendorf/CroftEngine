#pragma once

#include "gameplayrules.h"
#include "script/scriptengine.h"
#include "serialization/serialization_fwd.h"
#include "world/worldgeometry.h"

#include <boost/assert.hpp>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>

namespace loader::trx
{
class Glidos;
}

namespace engine::world
{
class World;
}

namespace engine::script
{
class LevelSequenceItem;
}

namespace engine
{
class Player;
class Presenter;
struct EngineConfig;

enum class RunResult
{
  NextLevel,
  TitleLevel,
  LaraHomeLevel,
  ExitGame,
  RequestLoad,
  RequestLevel,
  RestartLevel
};

struct SavegameMeta
{
  std::string filename;

  void serialize(const serialization::Serializer<SavegameMeta>& ser) const;
  void deserialize(const serialization::Deserializer<SavegameMeta>& ser);
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

inline std::filesystem::path makeMetaFilepath(const std::filesystem::path& path)
{
  auto metaPath = path;
  metaPath.replace_extension(".meta.yaml");
  return metaPath;
}

class Engine
{
public:
  explicit Engine(std::filesystem::path userDataPath,
                  const std::filesystem::path& engineDataPath,
                  const std::optional<std::string>& localOverride,
                  const std::string& gameflowId,
                  const glm::ivec2& resolution = {1280, 800});

  ~Engine();

  [[nodiscard]] const auto& getPresenter() const noexcept
  {
    BOOST_ASSERT(m_presenter != nullptr);
    return *m_presenter;
  }

  [[nodiscard]] auto& getPresenter() noexcept
  {
    BOOST_ASSERT(m_presenter != nullptr);
    return *m_presenter;
  }

  std::pair<RunResult, std::optional<size_t>> run(world::World& world, bool isCutscene, bool allowSave);
  std::pair<RunResult, std::optional<size_t>> runTitleMenu(world::World& world);

  [[nodiscard]] const std::string& getLocale() const noexcept
  {
    return m_locale;
  }

  [[nodiscard]] std::string getLocaleWithoutEncoding() const
  {
    if(const auto idx = m_locale.find('.'); idx != std::string::npos)
      return m_locale.substr(0, idx);
    return m_locale;
  }

  [[nodiscard]] std::filesystem::path getSavegameRootPath() const;
  [[nodiscard]] std::filesystem::path getSavegamePath(const std::optional<size_t>& slot) const;
  [[nodiscard]] std::filesystem::path getAssetDataPath() const;

  [[nodiscard]] const std::filesystem::path& getEngineDataPath() const noexcept
  {
    return m_engineDataPath;
  }

  std::pair<RunResult, std::optional<size_t>> runLevelSequenceItem(script::LevelSequenceItem& item,
                                                                   const std::shared_ptr<Player>& player,
                                                                   const std::shared_ptr<Player>& levelStartPlayer);
  std::pair<RunResult, std::optional<size_t>>
    runLevelSequenceItemFromSave(script::LevelSequenceItem& item,
                                 const std::optional<size_t>& slot,
                                 const std::shared_ptr<Player>& player,
                                 const std::shared_ptr<Player>& levelStartPlayer);

  [[nodiscard]] const auto& getGlidos() const noexcept
  {
    return m_glidos;
  }

  [[nodiscard]] std::optional<SavegameMeta> getSavegameMeta(const std::filesystem::path& filename) const;
  [[nodiscard]] std::optional<SavegameMeta> getSavegameMeta(const std::optional<size_t>& slot) const;

  [[nodiscard]] auto& getEngineConfig() noexcept
  {
    return m_engineConfig;
  }

  [[nodiscard]] const auto& getEngineConfig() const noexcept
  {
    return m_engineConfig;
  }

  void registerWorld(world::World* world)
  {
    m_worlds.emplace(world);
  }

  void unregisterWorld(const gsl::not_null<world::World*>& world)
  {
    m_worlds.erase(world);
  }

  void applySettings();

  [[nodiscard]] const auto& getScriptEngine() const noexcept
  {
    return m_scriptEngine;
  }

  [[nodiscard]] const auto& getGameflowId() const noexcept
  {
    return m_gameflowId;
  }

  void onGameSavedOrLoaded();

  [[nodiscard]] std::shared_ptr<world::WorldGeometry>
    getWorldGeometryCacheOrReset(const std::filesystem::path& levelPath)
  {
    if(m_worldGeometryCache.first != levelPath)
    {
      m_worldGeometryCache.second.reset();
      return nullptr;
    }

    return m_worldGeometryCache.second;
  }

  void setWorldGeometryCache(const std::filesystem::path& key,
                             const gslu::nn_shared<world::WorldGeometry>& worldGeometry)
  {
    m_worldGeometryCache = {key, worldGeometry};
  }

  [[nodiscard]] const auto& getGameplayRules() const
  {
    return m_gameplayRules;
  }

  void setGameplayRules(const GameplayRules& rules)
  {
    m_gameplayRules = rules;
  }

private:
  void takeBugReport(world::World& world);

  std::filesystem::path m_userDataPath;
  std::filesystem::path m_engineDataPath;
  std::string m_gameflowId;
  script::ScriptEngine m_scriptEngine;
  gslu::nn_shared<EngineConfig> m_engineConfig;
  std::shared_ptr<Presenter> m_presenter;
  std::set<gsl::not_null<world::World*>> m_worlds;
  std::chrono::steady_clock::time_point m_saveReminderSince{};
  GameplayRules m_gameplayRules{};

  std::string m_locale;

  std::unique_ptr<loader::trx::Glidos> m_glidos;
  [[nodiscard]] std::unique_ptr<loader::trx::Glidos> loadGlidosPack() const;

  std::pair<std::filesystem::path, std::shared_ptr<world::WorldGeometry>> m_worldGeometryCache;
};
} // namespace engine
