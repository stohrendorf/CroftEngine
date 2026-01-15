#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "gslu.h"
#include "throttler.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <utility>

namespace ui
{
class Ui;
}
namespace engine::world
{
class World;
}

namespace menu
{
struct MenuDisplay;
}

namespace engine
{
struct SavegameMeta;
class GhostManager;
enum class LevelLoopResult : uint8_t;
class Presenter;

struct LevelLoop
{
  static constexpr auto BlendInDuration = core::LogicRate * 2_sec;
  std::shared_ptr<menu::MenuDisplay> menu;

  explicit LevelLoop(gslu::nn_shared<Presenter> presenter, const std::filesystem::path& userDataPath) noexcept;

  void tickGameLogic(world::World& world);
  bool tickCinematicLogic(world::World& world);

  void render(world::World& world,
              bool isCutscene,
              float interTickFactor,
              const std::optional<std::chrono::steady_clock::time_point>& saveReminderNext);
  std::optional<std::pair<LevelLoopResult, std::optional<size_t>>> getMenuResultAsGameLoopResult(
    world::World& world,
    GhostManager& ghostManager,
    bool ghostEnabled,
    bool noLoads,
    const std::function<std::optional<SavegameMeta>(const std::optional<size_t>&)>& getSavegameMeta);
  bool handleLaraDead(Presenter& presenter, world::World& world);

private:
  void renderGame(world::World& world,
                  const std::optional<std::chrono::steady_clock::time_point>& saveReminderNext,
                  ui::Ui& ui);
  void genericPreTick(world::World& world);
  void genericPostTick();

  gslu::nn_shared<Presenter> m_presenter;
  std::filesystem::path m_userDataPath;
  core::Tick m_runtime = 0_tick;
  core::Tick m_laraDeadTime = 0_tick;
  core::Tick m_ammoDisplayDuration = 0_tick;
  core::Tick m_bugReportSavedDuration = 0_tick;
};
} // namespace engine