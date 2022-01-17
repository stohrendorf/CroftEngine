#pragma once

#include "core/angle.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/items_tr1.h"
#include "engine/tracks_tr1.h"
#include "qs/quantity.h"

#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <pybind11/pytypes.h>
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
  TrackInfo(const std::string& name, size_t slot, bool looping)
      : name{name}
      , slot{slot}
      , looping{looping}
  {
  }

  std::filesystem::path name;
  size_t slot;
  bool looping;
};

class LevelSequenceItem
{
public:
  virtual ~LevelSequenceItem() = default;
  virtual std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) = 0;
  virtual std::pair<RunResult, std::optional<size_t>>
    runFromSave(Engine& /*engine*/, const std::optional<size_t>& /*slot*/, const std::shared_ptr<Player>& /*player*/);

  [[nodiscard]] virtual bool isLevel(const std::filesystem::path& path) const = 0;
};

class Level : public LevelSequenceItem
{
private:
  const std::string m_name;
  const size_t m_secrets;
  const bool m_useAlternativeLara;
  const std::unordered_map<std::string, std::string> m_titles;
  const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> m_itemTitles;
  const std::optional<TR1TrackId> m_track;
  const bool m_allowSave;
  const WeaponType m_defaultWeapon;

protected:
  [[nodiscard]] std::unique_ptr<world::World> loadWorld(Engine& engine, const std::shared_ptr<Player>& player);

public:
  explicit Level(std::string name,
                 size_t secrets,
                 bool useAlternativeLara,
                 std::unordered_map<std::string, std::string> titles,
                 std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
                 std::optional<TR1TrackId> track,
                 bool allowSave,
                 WeaponType defaultWeapon)
      : m_name{std::move(name)}
      , m_secrets{secrets}
      , m_useAlternativeLara{useAlternativeLara}
      , m_titles{std::move(titles)}
      , m_itemTitles{std::move(itemTitles)}
      , m_track{track}
      , m_allowSave{allowSave}
      , m_defaultWeapon{defaultWeapon}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) override;
  std::pair<RunResult, std::optional<size_t>>
    runFromSave(Engine& engine, const std::optional<size_t>& slot, const std::shared_ptr<Player>& player) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& path) const override;
};

class ModifyInventory : public LevelSequenceItem
{
private:
  const std::unordered_map<TR1ItemId, size_t> m_addInventory;
  const std::unordered_set<TR1ItemId> m_dropInventory;

public:
  explicit ModifyInventory(std::unordered_map<TR1ItemId, size_t> addInventory,
                           std::unordered_set<TR1ItemId> dropInventory)
      : m_addInventory{std::move(addInventory)}
      , m_dropInventory{std::move(dropInventory)}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};

class TitleMenu : public Level
{
public:
  TitleMenu(const std::string& name,
            bool useAlternativeLara,
            const std::unordered_map<std::string, std::string>& titles,
            const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
            std::optional<TR1TrackId> track);

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};

class Video : public LevelSequenceItem
{
private:
  const std::string m_name;

public:
  explicit Video(std::string name)
      : m_name{std::move(name)}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};

class Cutscene : public LevelSequenceItem
{
private:
  const std::string m_name;
  const TR1TrackId m_track;
  const bool m_flipRooms;
  const bool m_weaponSwap;
  const core::Angle m_cameraRot;
  const std::optional<core::Length> m_cameraPosX;
  const std::optional<core::Length> m_cameraPosZ;

public:
  explicit Cutscene(std::string name,
                    TR1TrackId track,
                    bool flipRooms,
                    bool weaponSwap,
                    float cameraRot,
                    int cameraPosX,
                    int cameraPosZ)
      : m_name{std::move(name)}
      , m_track{track}
      , m_flipRooms{flipRooms}
      , m_weaponSwap{weaponSwap}
      , m_cameraRot{core::angleFromDegrees(cameraRot)}
      , m_cameraPosX{cameraPosX}
      , m_cameraPosZ{cameraPosZ}
  {
  }

  explicit Cutscene(std::string name, TR1TrackId track, bool flipRooms, bool weaponSwap, float cameraRot)
      : m_name{std::move(name)}
      , m_track{track}
      , m_flipRooms{flipRooms}
      , m_weaponSwap{weaponSwap}
      , m_cameraRot{core::angleFromDegrees(cameraRot)}
      , m_cameraPosX{std::nullopt}
      , m_cameraPosZ{std::nullopt}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};

class SplashScreen : public LevelSequenceItem
{
private:
  const std::filesystem::path m_path;
  const int m_durationSeconds;

public:
  explicit SplashScreen(std::string path, int durationSeconds);

  ~SplashScreen() override;

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine, const std::shared_ptr<Player>& player) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};

class Gameflow final
{
public:
  explicit Gameflow(std::map<TR1ItemId, ObjectInfo*> objectInfos,
                    std::map<TR1TrackId, TrackInfo*> tracks,
                    std::vector<LevelSequenceItem*> levelSequence,
                    LevelSequenceItem* titleMenu,
                    std::string titleMenuBackdrop,
                    std::vector<LevelSequenceItem*> laraHome,
                    std::vector<LevelSequenceItem*> earlyBoot,
                    pybind11::dict cheats)
      : m_objectInfos{std::move(objectInfos)}
      , m_tracks{std::move(tracks)}
      , m_levelSequence{std::move(levelSequence)}
      , m_titleMenu{titleMenu}
      , m_titleMenuBackdrop{std::move(titleMenuBackdrop)}
      , m_laraHome{std::move(laraHome)}
      , m_earlyBoot{std::move(earlyBoot)}
      , m_cheats{std::move(cheats)}
  {
  }

  [[nodiscard]] const auto& getObjectInfos() const
  {
    return m_objectInfos;
  }

  [[nodiscard]] const auto& getLevelSequence() const
  {
    return m_levelSequence;
  }

  [[nodiscard]] const auto& getEarlyBoot() const
  {
    return m_earlyBoot;
  }

  [[nodiscard]] const auto& getTitleMenu() const
  {
    return m_titleMenu;
  }

  [[nodiscard]] const auto& getTitleMenuBackdrop() const
  {
    return m_titleMenuBackdrop;
  }

  [[nodiscard]] const auto& getLaraHome() const
  {
    return m_laraHome;
  }

  [[nodiscard]] const auto& getTracks() const
  {
    return m_tracks;
  }

  [[nodiscard]] bool isGodMode() const;
  [[nodiscard]] bool hasAllAmmoCheat() const;
  [[nodiscard]] pybind11::dict getCheatInventory() const;

private:
  std::map<TR1ItemId, ObjectInfo*> m_objectInfos;
  std::map<TR1TrackId, TrackInfo*> m_tracks;
  std::vector<LevelSequenceItem*> m_levelSequence;
  LevelSequenceItem* m_titleMenu;
  std::string m_titleMenuBackdrop;
  std::vector<LevelSequenceItem*> m_laraHome;
  std::vector<LevelSequenceItem*> m_earlyBoot;
  pybind11::dict m_cheats;
};
} // namespace engine::script
