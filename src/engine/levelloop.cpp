#include "levelloop.h"

#include "audioengine.h"
#include "cameracontroller.h"
#include "core/i18n.h"
#include "engine.h"
#include "engineconfig.h"
#include "ghostmanager.h"
#include "hid/inputhandler.h"
#include "levelloopresult.h"
#include "menu/menudisplay.h"
#include "objects/laraobject.h"
#include "player.h"
#include "presenter.h"
#include "render/material/materialmanager.h"
#include "render/scene/rendercontext.h"
#include "render/scene/translucency.h"
#include "soundeffects_tr1.h"
#include "ui/core.h"
#include "ui/ui.h"
#include "util/datetime.h"
#include "world/world.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/log/trivial.hpp>

namespace engine
{
namespace
{
void constructAmmoWidget(ui::Ui& ui,
                         const ui::TRFont& trFont,
                         const world::World& world,
                         core::Tick& ammoDisplayDuration)
{
  if(const auto handStatus = world.getObjectManager().getLara().getHandStatus();
     handStatus != objects::HandStatus::Combat)
  {
    ammoDisplayDuration = 0_tick;
  }
  else
  {
    static constexpr auto StaticDuration = core::LogicRate * 1_sec * 2 / 3;
    static constexpr auto TransitionDuration = core::LogicRate * 1_sec / 2;

    ammoDisplayDuration = std::min(ammoDisplayDuration + 1_tick, StaticDuration + TransitionDuration);

    auto drawAmmoText = [&ui, &world, &trFont](const float animFactor)
    {
      const auto& ammo = world.getPlayer().getInventory().getAmmo(world.getPlayer().selectedWeaponType);
      const auto text = ui::Text{ui::makeAmmoString(ammo.getDisplayString())};
      const auto margin = ui::FontHeight / 2 + glm::mix(ui::FontHeight, 0, animFactor);
      const auto targetPos
        = glm::ivec2{ui.getSize().x - ui::FontHeight - 1 - margin - text.getWidth(), ui::FontHeight * 2 + margin};
      const auto center = glm::ivec2{(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y * 30 / 100};
      const auto pos = glm::mix(center, targetPos, animFactor);
      drawBox(text, ui, pos, margin, gl::SRGBA8{0, 0, 0, glm::mix(128, 224, animFactor)});
      text.draw(ui, trFont, pos, 1, glm::mix(0.75f, 1.0f, animFactor));
    };

    switch(world.getPlayer().selectedWeaponType)
    {
    case WeaponType::None:
    case WeaponType::Pistols:
      break;
    case WeaponType::Shotgun:
    case WeaponType::Magnums:
    case WeaponType::Uzis:
      drawAmmoText(glm::smoothstep(
        0.0f,
        1.0f,
        std::clamp(
          (ammoDisplayDuration - StaticDuration).cast<float>() / TransitionDuration.cast<float>(), 0.0f, 1.0f)));
      break;
    }
  }
}

void constructBugReportMessage(ui::Ui& ui, const ui::TRFont& trFont)
{
  const auto text = ui::Text{/* translators: TR charmap encoding */ _("Bug Report Saved")};
  const auto pos = glm::ivec2{(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y / 2 - ui::FontHeight};
  text.draw(ui, trFont, pos);
}

void constructSaveReminder(ui::Ui& ui, const ui::TRFont& trFont)
{
  const auto text = ui::Text{/* translators: TR charmap encoding */ _("Save Game")};
  const auto pos = glm::ivec2{(ui.getSize().x - text.getWidth()) / 2, ui::FontHeight};
  text.draw(ui, trFont, pos);
}

void takeBugReport(const std::filesystem::path& userDataPath, world::World& world, const Presenter& presenter)
{
  const auto bugReports = userDataPath / "bugreports";
  if(!std::filesystem::is_directory(bugReports))
  {
    std::filesystem::create_directory(bugReports);
  }

  const auto bugReport = bugReports / util::getCurrentHumanReadableTimestamp();
  if(!std::filesystem::is_directory(bugReport))
  {
    std::filesystem::create_directory(bugReport);
  }

  for(std::filesystem::directory_iterator it{userDataPath}; it != std::filesystem::directory_iterator{}; ++it)
  {
    BOOST_LOG_TRIVIAL(info) << it->path() << "; " << it->path().filename().string();
    if(!it->is_regular_file() || !boost::algorithm::starts_with(it->path().filename().string(), "croftengine.")
       || !boost::algorithm::ends_with(it->path().filename().string(), ".log"))
      continue;

    std::filesystem::copy_file(it->path(), bugReport / it->path().filename());
  }

  auto img = presenter.takeScreenshot();
  img.savePng(bugReport / "screenshot.png", false);

  world.save(bugReport / "save.yaml");
}

void makeScreenshot(const Presenter& presenter, const std::filesystem::path& userDataPath)
{
  auto img = presenter.takeScreenshot();

  const auto screenshotsDir = userDataPath / "screenshots";
  if(!std::filesystem::is_directory(screenshotsDir))
    std::filesystem::create_directories(screenshotsDir);

  const auto filename = util::getCurrentHumanReadableTimestamp() + ".png";
  img.savePng(screenshotsDir / filename, false);
}
} // namespace

LevelLoop::LevelLoop(gslu::nn_shared<Presenter> presenter, const std::filesystem::path& userDataPath) noexcept
    : m_presenter{std::move(presenter)}
    , m_userDataPath{userDataPath}
{
}

void LevelLoop::genericPreTick(world::World& world)
{
  m_presenter->getInputHandler().update();

  if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::BugReport))
  {
    takeBugReport(m_userDataPath, world, *m_presenter);
    m_bugReportSavedDuration = core::LogicRate * 5_sec;
  }

  if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
  {
    makeScreenshot(*m_presenter, m_userDataPath);
  }
}

void LevelLoop::genericPostTick()
{
  if(m_runtime < BlendInDuration)
    m_runtime += 1_tick;

  if(m_bugReportSavedDuration != 0_tick)
    m_bugReportSavedDuration -= 1_tick;
}

void LevelLoop::tickGameLogic(world::World& world)
{
  genericPreTick(world);

  if(menu != nullptr)
  {
    // Update camera's previous state to match current so interpolation doesn't jiggle
    world.getCameraController().updateGameLogic(true);
    menu->tick(world);
  }

  genericPostTick();
}

bool LevelLoop::tickCinematicLogic(world::World& world)
{
  genericPreTick(world);

  if(!world.tickCinematic())
    return false;

  world.updateLogicWorldState(false);

  genericPostTick();
  return true;
}

void LevelLoop::renderGame(world::World& world,
                           const std::optional<std::chrono::steady_clock::time_point>& saveReminderNext,
                           ui::Ui& ui)
{
  if(menu != nullptr)
  {
    const auto override = m_presenter->getRenderSystem().getMaterialManager().overrideInterTickFactor();
    m_presenter->renderWorldGeometryFramebuffers(
      world.getRooms(), world.getCameraController(), world.getCameraController().traceWaterSurfacePortals(), world);
  }
  else
  {
    m_presenter->renderWorldGeometryFramebuffers(
      world.getRooms(), world.getCameraController(), world.getCameraController().traceWaterSurfacePortals(), world);
  }

  if(menu != nullptr)
  {
    ui::Ui menuBackground{m_presenter->getRenderSystem().getMaterialManager().getUi(),
                          world.getWorldGeometry().getPalette(),
                          m_presenter->getUiViewport()};
    menuBackground.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, ui::DefaultBackgroundAlpha});
    m_presenter->renderUiToBackbuffer(menuBackground, 1);

    menu->constructUi(ui, world);
    menu->renderObjectsToFramebuffer(world);
    m_presenter->inBackbuffer(
      [this, &world]
      {
        const render::scene::RenderContext context{
          render::material::RenderMode::FullOpaque, std::nullopt, render::scene::Translucency::Opaque};
        menu->renderFramebuffer(world);
      });
  }
  else
  {
    constructAmmoWidget(ui, m_presenter->getTrFont(), world, m_ammoDisplayDuration);
    if(m_bugReportSavedDuration != 0_tick)
    {
      constructBugReportMessage(ui, m_presenter->getTrFont());
    }

    if(saveReminderNext.has_value() && std::chrono::steady_clock::now() >= *saveReminderNext)
    {
      constructSaveReminder(ui, m_presenter->getTrFont());
    }

    m_presenter->constructBars(ui,
                               world.getWorldGeometry().getPalette(),
                               world.getObjectManager(),
                               world.getEngine().getEngineConfig()->pulseLowHealthHealthBar);

    world.constructPickupWidgets(ui);
  }
}

void LevelLoop::render(world::World& world,
                       const bool isCutscene,
                       const float interTickFactor,
                       const std::optional<std::chrono::steady_clock::time_point>& saveReminderNext)
{
  ui::Ui ui{m_presenter->getRenderSystem().getMaterialManager().getUi(),
            world.getWorldGeometry().getPalette(),
            m_presenter->getUiViewport()};

  // When menu is open, freeze object transforms but keep camera interpolating
  const float objectInterTickFactor = (menu != nullptr) ? 0.0f : interTickFactor;

  world.getCameraController().interpolateCameraTransform(interTickFactor);
  world.getObjectManager().interpolateTransforms(objectInterTickFactor);

  if(const auto lara = world.getObjectManager().getLaraPtr())
    lara->m_state.location.room->node->setVisible(true);

  if(isCutscene)
  {
    world.renderCinematic();
  }
  else
  {
    renderGame(world, saveReminderNext, ui);
  }

  if(const float blackAlpha
     = m_runtime < BlendInDuration ? 1.0f - (m_runtime.cast<float>() / BlendInDuration.cast<float>()) : 0.0f;
     blackAlpha > 0)
  {
    ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, gsl_lite::narrow_cast<uint8_t>(255 * blackAlpha)});
  }

  m_presenter->renderUiToBackbuffer(ui, 1);
  m_presenter->swapBuffers();
}

std::optional<std::pair<LevelLoopResult, std::optional<size_t>>> LevelLoop::getMenuResultAsGameLoopResult(
  world::World& world,
  GhostManager& ghostManager,
  const bool ghostEnabled,
  const bool noLoads,
  const std::function<std::optional<SavegameMeta>(const std::optional<size_t>&)>& getSavegameMeta)
{
  gsl_Assert(menu != nullptr);

  switch(menu->result)
  {
  case menu::MenuResult::None:
    break;
  case menu::MenuResult::Closed:
    menu.reset();
    break;
  case menu::MenuResult::ExitToTitle:
    if(ghostEnabled)
      ghostManager.askGhostSave(*m_presenter, world);
    return std::pair{LevelLoopResult::TitleLevel, std::nullopt};
  case menu::MenuResult::ExitGame:
    if(ghostEnabled)
      ghostManager.askGhostSave(*m_presenter, world);
    return std::pair{LevelLoopResult::ExitGame, std::nullopt};
  case menu::MenuResult::NewGame:
    BOOST_THROW_EXCEPTION(std::runtime_error("unexpected menu result"));
  case menu::MenuResult::RestartLevel:
    if(ghostEnabled)
      ghostManager.askGhostSave(*m_presenter, world);
    return std::pair{LevelLoopResult::RestartLevel, std::nullopt};
  case menu::MenuResult::LaraHome:
    BOOST_THROW_EXCEPTION(std::runtime_error("unexpected menu result"));
  case menu::MenuResult::RequestLoad:
    if(getSavegameMeta(menu->requestLoad).has_value())
    {
      if(ghostEnabled)
        ghostManager.askGhostSave(*m_presenter, world);
      return std::pair{LevelLoopResult::RequestLoad, menu->requestLoad};
    }
    break;
  case menu::MenuResult::RequestLevel:
    BOOST_THROW_EXCEPTION(std::runtime_error("unexpected menu result"));
  }

  if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
  {
    if(noLoads)
    {
      if(auto lara = world.getObjectManager().getLaraPtr(); lara != nullptr)
      {
        lara->playSoundEffect(TR1SoundEffect::LaraNo);
      }
    }
    else if(getSavegameMeta(std::nullopt).has_value())
    {
      return std::pair{LevelLoopResult::RequestLoad, std::nullopt};
    }
  }

  return std::nullopt;
}

bool LevelLoop::handleLaraDead(Presenter& presenter, world::World& world)
{
  world.getAudioEngine().setMusicGain(0);
  m_laraDeadTime += 1_tick;
  if(m_laraDeadTime < core::LogicRate * 10_sec
     && (m_laraDeadTime < core::LogicRate * 2_sec || !presenter.getInputHandler().hasAnyAction()))
  {
    return false;
  }

  menu = std::make_shared<menu::MenuDisplay>(
    menu::InventoryMode::DeathMode, menu::SaveGamePageMode::Restart, false, world, presenter.getRenderViewport());
  presenter.getInputHandler().update();
  return true;
}
} // namespace engine