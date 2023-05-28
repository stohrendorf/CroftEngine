#pragma once

#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/items_tr1.h"
#include "engine/tracks_tr1.h"
#include "qs/quantity.h"

#include <boost/throw_exception.hpp>
#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <pybind11/pytypes.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace engine
{
enum class RunResult;
enum class WeaponType;
class Engine;
class Player;
} // namespace engine

namespace engine::world
{
class World;
}

namespace loader::file::level
{
enum class Game;
}

namespace engine::script
{
struct ObjectInfo
{
  bool ai_agent = false;
  core::Length::type radius = 10;
  core::Health::type hit_points = -16384;
  core::Length::type pivot_length = 0;
  int target_update_chance = 0;
  core::Length::type step_limit = core::QuarterSectorSize.get();
  core::Length::type drop_limit = -core::QuarterSectorSize.get();
  core::Length::type fly_limit = 0;
  bool cannot_visit_blocked = true;
  bool cannot_visit_blockable = false;
};

struct TrackInfo
{
  TrackInfo(const std::vector<std::string>& paths, size_t slot, bool looping, uint32_t fadeDurationSeconds)
      : slot{slot}
      , looping{looping}
      , fadeDurationSeconds{fadeDurationSeconds}
  {
    if(paths.empty())
      BOOST_THROW_EXCEPTION(std::invalid_argument("paths is empty"));

    for(const auto& path : paths)
      this->paths.emplace_back(path);
  }

  std::vector<std::filesystem::path> paths;
  size_t slot;
  bool looping;
  uint32_t fadeDurationSeconds;

  [[nodiscard]] std::vector<std::filesystem::path> getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const;
  [[nodiscard]] std::filesystem::path getFirstValidAlternative(const std::filesystem::path& dataRoot) const;
};

class LevelSequenceItem
{
public:
  virtual ~LevelSequenceItem() = default;
  virtual std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                          const std::shared_ptr<Player>& player,
                                                          const std::shared_ptr<Player>& levelStartPlayer)
    = 0;
  virtual std::pair<RunResult, std::optional<size_t>> runFromSave(const gsl::not_null<Engine*>& /*engine*/,
                                                                  const std::optional<size_t>& /*slot*/,
                                                                  const std::shared_ptr<Player>& /*player*/,
                                                                  const std::shared_ptr<Player>& /*levelStartPlayer*/);

  [[nodiscard]] virtual bool isLevel(const std::filesystem::path& path) const = 0;
  [[nodiscard]] virtual std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const = 0;
};

class Level : public LevelSequenceItem
{
private:
  std::string m_name;
  bool m_useAlternativeLara;
  std::unordered_map<std::string, std::string> m_titles;
  std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> m_itemTitles;
  std::optional<TR1TrackId> m_ambient;
  bool m_allowSave;
  WeaponType m_defaultWeapon;
  glm::vec3 m_waterColor;
  float m_waterDensity;
  std::optional<std::string> m_alternativeSplashscreen;
  loader::file::level::Game m_game;

protected:
  [[nodiscard]] std::unique_ptr<world::World> loadWorld(const gsl::not_null<Engine*>& engine,
                                                        const std::shared_ptr<Player>& player,
                                                        const std::shared_ptr<Player>& levelStartPlayer,
                                                        bool fromSave);

public:
  static constexpr auto DefaultWaterDensity = 0.2f;
  static constexpr auto DefaultWaterColor = std::tuple{0.0f, 0.462f, 0.494f};

  explicit Level(std::string name,
                 bool useAlternativeLara,
                 std::unordered_map<std::string, std::string> titles,
                 std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
                 std::optional<TR1TrackId> ambient,
                 bool allowSave,
                 WeaponType defaultWeapon,
                 std::tuple<float, float, float> waterColor,
                 float waterDensity,
                 std::optional<std::string> alternativeSplashscreen,
                 loader::file::level::Game game)
      : m_name{std::move(name)}
      , m_useAlternativeLara{useAlternativeLara}
      , m_titles{std::move(titles)}
      , m_itemTitles{std::move(itemTitles)}
      , m_ambient{ambient}
      , m_allowSave{allowSave}
      , m_defaultWeapon{defaultWeapon}
      , m_waterColor{std::get<0>(waterColor), std::get<1>(waterColor), std::get<2>(waterColor)}
      , m_waterDensity{waterDensity}
      , m_alternativeSplashscreen{std::move(alternativeSplashscreen)}
      , m_game{game}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;
  std::pair<RunResult, std::optional<size_t>> runFromSave(const gsl::not_null<Engine*>& engine,
                                                          const std::optional<size_t>& slot,
                                                          const std::shared_ptr<Player>& player,
                                                          const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& path) const override;

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const override;

  [[nodiscard]] std::filesystem::path getFilepath() const;
  [[nodiscard]] const auto& getTitles() const noexcept
  {
    return m_titles;
  }
};

class ModifyInventory : public LevelSequenceItem
{
private:
  std::unordered_map<TR1ItemId, size_t> m_addInventory;
  std::unordered_set<TR1ItemId> m_dropInventory;

public:
  explicit ModifyInventory(std::unordered_map<TR1ItemId, size_t> addInventory,
                           std::unordered_set<TR1ItemId> dropInventory)
      : m_addInventory{std::move(addInventory)}
      , m_dropInventory{std::move(dropInventory)}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& /*dataRoot*/) const override
  {
    return {};
  }
};

class TitleMenu : public Level
{
public:
  TitleMenu(const std::string& name,
            bool useAlternativeLara,
            const std::unordered_map<std::string, std::string>& titles,
            const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
            std::optional<TR1TrackId> ambient,
            loader::file::level::Game game);

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};

class Video : public LevelSequenceItem
{
private:
  std::vector<std::filesystem::path> m_paths;
  bool m_optional;

public:
  explicit Video(const std::vector<std::string>& paths, bool optional)
      : m_optional{optional}
  {
    if(paths.empty())
      BOOST_THROW_EXCEPTION(std::invalid_argument("paths is empty"));

    for(const auto& path : paths)
      m_paths.emplace_back(path);
  }

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const override;
};

class Cutscene : public LevelSequenceItem
{
private:
  std::string m_name;
  TR1TrackId m_track;
  bool m_flipRooms;
  bool m_weaponSwap;
  core::Angle m_cameraRot;
  std::optional<core::Length> m_cameraPosX;
  std::optional<core::Length> m_cameraPosZ;
  loader::file::level::Game m_game;

public:
  explicit Cutscene(std::string name,
                    TR1TrackId track,
                    bool flipRooms,
                    bool weaponSwap,
                    float cameraRot,
                    int cameraPosX,
                    int cameraPosZ,
                    loader::file::level::Game game) noexcept
      : m_name{std::move(name)}
      , m_track{track}
      , m_flipRooms{flipRooms}
      , m_weaponSwap{weaponSwap}
      , m_cameraRot{core::angleFromDegrees(cameraRot)}
      , m_cameraPosX{cameraPosX}
      , m_cameraPosZ{cameraPosZ}
      , m_game{game}
  {
  }

  explicit Cutscene(std::string name,
                    TR1TrackId track,
                    bool flipRooms,
                    bool weaponSwap,
                    float cameraRot,
                    loader::file::level::Game game) noexcept
      : m_name{std::move(name)}
      , m_track{track}
      , m_flipRooms{flipRooms}
      , m_weaponSwap{weaponSwap}
      , m_cameraRot{core::angleFromDegrees(cameraRot)}
      , m_cameraPosX{std::nullopt}
      , m_cameraPosZ{std::nullopt}
      , m_game{game}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const override;
};

class SplashScreen : public LevelSequenceItem
{
private:
  std::filesystem::path m_path;
  int m_durationSeconds;
  int m_fadeInDurationSeconds;
  int m_fadeOutDurationSeconds;

public:
  explicit SplashScreen(std::string path, int durationSeconds, int fadeInDurationSeconds, int fadeOutDurationSeconds);

  ~SplashScreen() override;

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const override;
};

class PlayAudioSlot : public LevelSequenceItem
{
private:
  size_t m_slot;
  TR1TrackId m_track;

public:
  explicit PlayAudioSlot(size_t slot, TR1TrackId track) noexcept
      : m_slot{slot}
      , m_track{track}
  {
  }

  ~PlayAudioSlot() override = default;

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& /*dataRoot*/) const override
  {
    return {};
  }
};

class StopAudioSlot : public LevelSequenceItem
{
private:
  size_t m_slot;

public:
  explicit StopAudioSlot(size_t slot) noexcept
      : m_slot{slot}
  {
  }

  ~StopAudioSlot() override = default;

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& /*dataRoot*/) const override
  {
    return {};
  }
};

class ResetSoundEngine : public LevelSequenceItem
{
public:
  explicit ResetSoundEngine() = default;

  ~ResetSoundEngine() override = default;

  std::pair<RunResult, std::optional<size_t>> run(const gsl::not_null<Engine*>& engine,
                                                  const std::shared_ptr<Player>& player,
                                                  const std::shared_ptr<Player>& levelStartPlayer) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }

  [[nodiscard]] std::vector<std::filesystem::path>
    getFilepathsIfInvalid(const std::filesystem::path& /*dataRoot*/) const override
  {
    return {};
  }
};

class Gameflow final
{
public:
  explicit Gameflow(std::map<TR1ItemId, std::shared_ptr<ObjectInfo>> objectInfos,
                    std::map<TR1TrackId, std::shared_ptr<TrackInfo>> tracks,
                    std::vector<std::shared_ptr<LevelSequenceItem>> levelSequence,
                    std::shared_ptr<LevelSequenceItem> titleMenu,
                    std::string titleMenuBackdrop,
                    std::vector<std::shared_ptr<LevelSequenceItem>> laraHome,
                    std::vector<std::shared_ptr<LevelSequenceItem>> earlyBoot,
                    pybind11::dict cheats,
                    std::string assetRoot)
      : m_objectInfos{std::move(objectInfos)}
      , m_tracks{std::move(tracks)}
      , m_levelSequence{std::move(levelSequence)}
      , m_titleMenu{std::move(titleMenu)}
      , m_titleMenuBackdrop{std::move(titleMenuBackdrop)}
      , m_laraHome{std::move(laraHome)}
      , m_earlyBoot{std::move(earlyBoot)}
      , m_cheats{std::move(cheats)}
      , m_assetRoot{std::move(assetRoot)}
  {
  }

  [[nodiscard]] const auto& getObjectInfos() const noexcept
  {
    return m_objectInfos;
  }

  [[nodiscard]] const auto& getLevelSequence() const noexcept
  {
    return m_levelSequence;
  }

  [[nodiscard]] const auto& getEarlyBoot() const noexcept
  {
    return m_earlyBoot;
  }

  [[nodiscard]] const auto& getTitleMenu() const noexcept
  {
    return m_titleMenu;
  }

  [[nodiscard]] const auto& getTitleMenuBackdrop() const noexcept
  {
    return m_titleMenuBackdrop;
  }

  [[nodiscard]] const auto& getLaraHome() const noexcept
  {
    return m_laraHome;
  }

  [[nodiscard]] const auto& getTracks() const noexcept
  {
    return m_tracks;
  }

  [[nodiscard]] const auto& getAssetRoot() const noexcept
  {
    return m_assetRoot;
  }

  [[nodiscard]] bool isGodMode() const;
  [[nodiscard]] bool hasAllAmmoCheat() const;
  [[nodiscard]] pybind11::dict getCheatInventory() const;

  [[nodiscard]] std::vector<std::filesystem::path> getInvalidFilepaths(const std::filesystem::path& dataRoot) const;

  [[nodiscard]] std::map<std::filesystem::path, std::unordered_map<std::string, std::string>>
    getLevelFilepathsTitles() const;

private:
  std::map<TR1ItemId, std::shared_ptr<ObjectInfo>> m_objectInfos;
  std::map<TR1TrackId, std::shared_ptr<TrackInfo>> m_tracks;
  std::vector<std::shared_ptr<LevelSequenceItem>> m_levelSequence;
  std::shared_ptr<LevelSequenceItem> m_titleMenu;
  std::string m_titleMenuBackdrop;
  std::vector<std::shared_ptr<LevelSequenceItem>> m_laraHome;
  std::vector<std::shared_ptr<LevelSequenceItem>> m_earlyBoot;
  pybind11::dict m_cheats;
  std::string m_assetRoot;
};
} // namespace engine::script
