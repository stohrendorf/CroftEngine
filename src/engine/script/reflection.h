#pragma once

#include "audio/tracktype.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/items_tr1.h"
#include "engine/tracks_tr1.h"
#include "qs/quantity.h"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

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
  TrackInfo(core::SoundEffectId::type id, audio::TrackType type)
      : id{id}
      , type{type}
  {
  }

  core::SoundEffectId id;
  audio::TrackType type;
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
  const std::unordered_map<TR1ItemId, size_t> m_inventory;
  const std::unordered_set<TR1ItemId> m_dropInventory;
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
                 std::unordered_map<TR1ItemId, size_t> inventory,
                 std::unordered_set<TR1ItemId> dropInventory,
                 std::optional<TR1TrackId> track,
                 bool allowSave,
                 WeaponType defaultWeapon)
      : m_name{std::move(name)}
      , m_secrets{secrets}
      , m_useAlternativeLara{useAlternativeLara}
      , m_titles{std::move(titles)}
      , m_itemTitles{std::move(itemTitles)}
      , m_inventory{std::move(inventory)}
      , m_dropInventory{std::move(dropInventory)}
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

class TitleMenu : public Level
{
public:
  TitleMenu(const std::string& name,
            bool useAlternativeLara,
            const std::unordered_map<std::string, std::string>& titles,
            const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
            const std::unordered_map<TR1ItemId, size_t>& inventory,
            const std::unordered_set<TR1ItemId>& dropInventory,
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
} // namespace engine::script
