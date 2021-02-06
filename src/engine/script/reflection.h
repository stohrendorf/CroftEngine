#pragma once

#include "audio/tracktype.h"
#include "core/angle.h"
#include "core/id.h"
#include "core/units.h"
#include "engine/tracks_tr1.h"

#include <filesystem>

namespace engine
{
enum class RunResult;
class Engine;
} // namespace engine

namespace engine::script
{
struct ObjectInfo
{
  bool ai_agent = false;
  core::Length::type radius = 10;
  core::Health::type hit_points = -16384;
  core::Length::type pivot_length = 0;
  int target_update_chance = 0;
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
  virtual std::pair<RunResult, std::optional<size_t>> run(Engine& engine) = 0;
  virtual std::pair<RunResult, std::optional<size_t>> runFromSave(Engine& /*engine*/,
                                                                  const std::optional<size_t>& /*slot*/)
  {
    BOOST_LOG_TRIVIAL(error) << "Cannot run from save";
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot run from save"));
  }

  [[nodiscard]] virtual bool isLevel(const std::filesystem::path& path) const = 0;
};

class Level : public LevelSequenceItem
{
private:
  const std::string m_name;
  const int m_secrets;
  const bool m_useAlternativeLara;
  const std::unordered_map<std::string, std::string> m_titles;
  const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> m_itemTitles;
  const std::unordered_map<TR1ItemId, size_t> m_inventory;
  const TR1TrackId m_track;
  const bool m_allowSave;

protected:
  [[nodiscard]] std::unique_ptr<engine::World> loadWorld(Engine& engine);

public:
  explicit Level(std::string name,
                 int secrets,
                 bool useAlternativeLara,
                 std::unordered_map<std::string, std::string> titles,
                 std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>> itemTitles,
                 std::unordered_map<TR1ItemId, size_t> inventory,
                 TR1TrackId track,
                 bool allowSave)
      : m_name{std::move(name)}
      , m_secrets{secrets}
      , m_useAlternativeLara{useAlternativeLara}
      , m_titles{std::move(titles)}
      , m_itemTitles{std::move(itemTitles)}
      , m_inventory{std::move(inventory)}
      , m_track{track}
      , m_allowSave{allowSave}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine) override;
  std::pair<RunResult, std::optional<size_t>> runFromSave(Engine& engine, const std::optional<size_t>& slot) override;

  bool isLevel(const std::filesystem::path& path) const override;
};

class TitleMenu : public Level
{
public:
  TitleMenu(const std::string& name,
            int secrets,
            bool useAlternativeLara,
            const std::unordered_map<std::string, std::string>& titles,
            const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
            const std::unordered_map<TR1ItemId, size_t>& inventory,
            TR1TrackId track)
      : Level{name, secrets, useAlternativeLara, titles, itemTitles, inventory, track, false}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine) override;

  bool isLevel(const std::filesystem::path& /*path*/) const override
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

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine) override;

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
  const bool m_gunSwap;
  const core::Angle m_cameraRot;
  const std::optional<core::Length> m_cameraPosX;
  const std::optional<core::Length> m_cameraPosZ;

public:
  explicit Cutscene(
    std::string name, TR1TrackId track, bool flipRooms, bool gunSwap, float cameraRot, int cameraPosX, int cameraPosZ)
      : m_name{std::move(name)}
      , m_track{track}
      , m_flipRooms{flipRooms}
      , m_gunSwap{gunSwap}
      , m_cameraRot{core::angleFromDegrees(cameraRot)}
      , m_cameraPosX{cameraPosX}
      , m_cameraPosZ{cameraPosZ}
  {
  }

  explicit Cutscene(std::string name, TR1TrackId track, bool flipRooms, bool gunSwap, float cameraRot)
      : m_name{std::move(name)}
      , m_track{track}
      , m_flipRooms{flipRooms}
      , m_gunSwap{gunSwap}
      , m_cameraRot{core::angleFromDegrees(cameraRot)}
      , m_cameraPosX{std::nullopt}
      , m_cameraPosZ{std::nullopt}
  {
  }

  std::pair<RunResult, std::optional<size_t>> run(Engine& engine) override;

  [[nodiscard]] bool isLevel(const std::filesystem::path& /*path*/) const override
  {
    return false;
  }
};
} // namespace engine::script
