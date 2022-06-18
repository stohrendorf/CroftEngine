#include "engine.h"

#include "audio/soundengine.h"
#include "audio/streamvoice.h"
#include "core/i18n.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/audioengine.h"
#include "engine/audiosettings.h"
#include "engine/cameracontroller.h"
#include "engine/displaysettings.h"
#include "engine/engineconfig.h"
#include "engine/ghosting/ghostmodel.h"
#include "engine/inventory.h"
#include "engine/objectmanager.h"
#include "engine/objects/objectstate.h"
#include "engine/weapontype.h"
#include "engine/world/room.h"
#include "ghosting/ghost.h"
#include "ghostmanager.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "loader/trx/trx.h"
#include "menu/menudisplay.h"
#include "objects/laraobject.h"
#include "player.h"
#include "presenter.h"
#include "qs/qs.h"
#include "render/rendersettings.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "script/reflection.h"
#include "script/scriptengine.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "ui/core.h"
#include "ui/levelstats.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "ui/widgets/messagebox.h"
#include "util/helpers.h"
#include "world/world.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <exception>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/framebuffer.h>
#include <gl/glad_init.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <gslu.h>
#include <iosfwd>
#include <locale>
#include <pybind11/eval.h>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

namespace engine
{
namespace
{
const gsl::czstring QuicksaveFilename = "quicksave.yaml";

void drawAmmoWidget(ui::Ui& ui, const ui::TRFont& trFont, const world::World& world, core::Frame& ammoDisplayDuration)
{
  if(const auto handStatus = world.getObjectManager().getLara().getHandStatus();
     handStatus != engine::objects::HandStatus::Combat)
  {
    ammoDisplayDuration = 0_frame;
  }
  else
  {
    static constexpr auto StaticDuration = (core::FrameRate * 1_sec * 2 / 3).cast<core::Frame>();
    static constexpr auto TransitionDuration = (core::FrameRate * 1_sec / 2).cast<core::Frame>();

    ammoDisplayDuration = std::min(ammoDisplayDuration + 1_frame, StaticDuration + TransitionDuration);

    auto drawAmmoText = [&ui, &world, &trFont](float bias)
    {
      const auto& ammo = world.getPlayer().getInventory().getAmmo(world.getPlayer().selectedWeaponType);
      auto text = ui::Text{ui::makeAmmoString(ammo.getDisplayString())};
      const auto margin = ui::FontHeight / 2 + glm::mix(ui::FontHeight, 0, bias);
      const auto targetPos
        = glm::ivec2{ui.getSize().x - ui::FontHeight - 1 - margin - text.getWidth(), ui::FontHeight * 2 + margin};
      const auto center = (ui.getSize() - glm::ivec2{text.getWidth(), ui::FontHeight}) / 2;
      const auto pos = glm::mix(center, targetPos, bias);
      drawBox(text, ui, pos, margin, gl::SRGBA8{0, 0, 0, glm::mix(128, 224, bias)});
      text.draw(ui, trFont, pos, 1, glm::mix(0.75f, 1.0f, bias));
    };

    switch(world.getPlayer().selectedWeaponType)
    {
    case engine::WeaponType::None:
    case engine::WeaponType::Pistols:
      break;
    case engine::WeaponType::Shotgun:
    case engine::WeaponType::Magnums:
    case engine::WeaponType::Uzis:
      drawAmmoText(glm::smoothstep(
        0.0f,
        1.0f,
        std::clamp(
          (ammoDisplayDuration - StaticDuration).cast<float>() / TransitionDuration.cast<float>(), 0.0f, 1.0f)));
      break;
    }
  }
}

void drawBugReportMessage(ui::Ui& ui, const ui::TRFont& trFont)
{
  auto text = ui::Text{/* translators: TR charmap encoding */ _("Bug Report Saved")};
  const auto pos = glm::ivec2{(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y / 2 - ui::FontHeight};
  text.draw(ui, trFont, pos);
}

bool showLevelStats(const std::shared_ptr<Presenter>& presenter, world::World& world)
{
  static constexpr const auto BlendDuration = 30_frame;
  auto currentBlendDuration = 0_frame;

  Throttler throttler;
  while(true)
  {
    throttler.wait();
    if(presenter->shouldClose())
    {
      return false;
    }

    if(!presenter->preFrame())
    {
      continue;
    }

    ui::Ui ui{presenter->getMaterialManager()->getUi(), world.getPalette(), presenter->getUiViewport()};
    ui::LevelStats stats{world.getTitle(), world.getTotalSecrets(), world.getPlayerPtr(), presenter};
    stats.draw(ui);

    {
      const auto portals = world.getCameraController().update();
      if(const auto lara = world.getObjectManager().getLaraPtr())
        lara->m_state.location.room->node->setVisible(true);
      presenter->renderWorld(world.getRooms(), world.getCameraController(), portals);
    }
    presenter->renderScreenOverlay();

    if(currentBlendDuration < BlendDuration)
      currentBlendDuration += 1_frame;

    presenter->renderUi(ui, currentBlendDuration.cast<float>() / BlendDuration.cast<float>());
    presenter->updateSoundEngine();
    presenter->swapBuffers();

    if(presenter->getInputHandler().hasDebouncedAction(hid::Action::Action))
      break;
  }

  return true;
}

std::string getCurrentHumanReadableTimestamp()
{
  auto time = std::time(nullptr);
#ifdef WIN32
  struct tm localTimeData
  {
  };
  Expects(localtime_s(&localTimeData, &time) == 0);
  auto localTime = &localTimeData;
#else
  // NOLINTNEXTLINE(concurrency-mt-unsafe)
  auto localTime = std::localtime(&time);
#endif
  return (boost::format("%04d-%02d-%02d %02d-%02d-%02d") % (localTime->tm_year + 1900) % (localTime->tm_mon + 1)
          % localTime->tm_mday % localTime->tm_hour % localTime->tm_min % localTime->tm_sec)
    .str();
}
} // namespace

Engine::Engine(std::filesystem::path userDataPath,
               const std::filesystem::path& engineDataPath,
               const std::optional<std::string>& localOverride,
               const std::string& gameflowId,
               const glm::ivec2& resolution)
    : m_userDataPath{std::move(userDataPath)}
    , m_engineDataPath{engineDataPath}
    , m_gameflowId{gameflowId}
    , m_scriptEngine{engineDataPath / "gameflows" / gameflowId}
{
  {
    const auto invalid = m_scriptEngine.getGameflow().getInvalidFilepaths(*this);
    for(const auto& path : invalid)
      BOOST_LOG_TRIVIAL(fatal) << "Missing required game file " << path;
    if(!invalid.empty())
      BOOST_THROW_EXCEPTION(std::runtime_error("Missing required game file(s)"));
  }

  m_locale = std::use_facet<boost::locale::info>(boost::locale::generator()("")).name();
  BOOST_LOG_TRIVIAL(info) << "Detected user's locale is " << m_locale;
  if(localOverride.has_value())
  {
    m_locale = *localOverride;
    BOOST_LOG_TRIVIAL(info) << "Locale override is " << m_locale;
  }

  core::setLocale(std::filesystem::absolute(m_engineDataPath / "po"), m_locale);

  m_engineConfig = std::make_unique<EngineConfig>();
  if(std::filesystem::is_regular_file(m_userDataPath / "config.yaml"))
  {
    serialization::YAMLDocument<true> doc{m_userDataPath / "config.yaml"};
    doc.load("config", *m_engineConfig, *m_engineConfig);
  }

  m_presenter = std::make_shared<Presenter>(m_engineDataPath, resolution);
  if(gl::hasAnisotropicFilteringExtension()
     && m_engineConfig->renderSettings.anisotropyLevel > gl::getMaxAnisotropyLevel())
  {
    m_engineConfig->renderSettings.anisotropyLevel = gsl::narrow<uint32_t>(std::llround(gl::getMaxAnisotropyLevel()));
  }

  applySettings();
  m_presenter->getInputHandler().setMappings(m_engineConfig->inputMappings);
  m_glidos = loadGlidosPack();
}

Engine::~Engine()
{
  serialization::YAMLDocument<false> doc{m_userDataPath / "config.yaml"};
  doc.save("config", *m_engineConfig, *m_engineConfig);
  doc.write();
}

std::pair<RunResult, std::optional<size_t>> Engine::run(world::World& world, bool isCutscene, bool allowSave)
{
  gl::Framebuffer::unbindAll();
  if(!isCutscene)
  {
    world.getObjectManager().getLara().m_state.health = world.getPlayer().laraHealth;
    world.getObjectManager().getLara().initWeaponAnimData();
  }

  const bool godMode = m_scriptEngine.getGameflow().isGodMode();
  const bool allAmmoCheat = m_scriptEngine.getGameflow().hasAllAmmoCheat();

  applySettings();
  std::shared_ptr<menu::MenuDisplay> menu;
  Throttler throttler;
  core::Frame laraDeadTime = 0_frame;

  core::Frame runtime = 0_frame;
  static constexpr auto BlendInDuration = (core::FrameRate * 2_sec).cast<core::Frame>();
  core::Frame ammoDisplayDuration = 0_frame;
  core::Frame bugReportSavedDuration = 0_frame;

  const auto ghostRoot = m_userDataPath / "ghosts" / m_gameflowId;
  std::filesystem::create_directories(ghostRoot);
  GhostManager ghostManager{ghostRoot / (world.getLevelFilename().stem().replace_extension(".rec")), world};

  while(true)
  {
    ghostManager.model->setVisible(m_engineConfig->displaySettings.ghost);

    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitApp, std::nullopt};
    }

    if(world.levelFinished())
    {
      if(!isCutscene && allowSave)
      {
        if(!showLevelStats(m_presenter, world))
          return {RunResult::ExitApp, std::nullopt};

        if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
          return {RunResult::ExitApp, std::nullopt};
      }

      return {RunResult::NextLevel, std::nullopt};
    }

    throttler.wait();
    if(!m_presenter->preFrame())
    {
      continue;
    }

    if(menu != nullptr)
    {
      {
        const auto portals = world.getCameraController().update();
        if(const auto lara = world.getObjectManager().getLaraPtr())
          lara->m_state.location.room->node->setVisible(true);
        m_presenter->renderWorld(world.getRooms(), world.getCameraController(), portals);
      }
      m_presenter->updateSoundEngine();
      m_presenter->renderScreenOverlay();
      ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette(), m_presenter->getUiViewport()};
      ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, 224});
      m_presenter->renderUi(ui, 1);
      menu->display(ui, world);
      m_presenter->renderUi(ui, 1);
      m_presenter->swapBuffers();
      switch(menu->result)
      {
      case menu::MenuResult::None:
        break;
      case menu::MenuResult::Closed:
        menu.reset();
        throttler.reset();
        break;
      case menu::MenuResult::ExitToTitle:
        if(allowSave)
        {
          if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
            return {RunResult::ExitApp, std::nullopt};
        }
        return {RunResult::TitleLevel, std::nullopt};
      case menu::MenuResult::ExitGame:
        if(allowSave)
        {
          if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
            return {RunResult::ExitApp, std::nullopt};
        }
        return {RunResult::ExitApp, std::nullopt};
      case menu::MenuResult::NewGame:
        if(allowSave)
        {
          if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
            return {RunResult::ExitApp, std::nullopt};
        }
        return {RunResult::NextLevel, std::nullopt};
      case menu::MenuResult::RestartLevel:
        return {RunResult::RestartLevel, std::nullopt};
      case menu::MenuResult::LaraHome:
        return {RunResult::LaraHomeLevel, std::nullopt};
      case menu::MenuResult::RequestLoad:
        if(getSavegameMeta(menu->requestLoad).has_value())
        {
          if(allowSave)
          {
            if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
              return {RunResult::ExitApp, std::nullopt};
          }
          return {RunResult::RequestLoad, menu->requestLoad};
        }
      }

      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
      {
        if(getSavegameMeta(std::nullopt).has_value())
        {
          return {RunResult::RequestLoad, std::nullopt};
        }
      }
      continue;
    }

    if(!isCutscene)
    {
      if(world.getObjectManager().getLara().isDead())
      {
        world.getAudioEngine().setMusicGain(0);
        laraDeadTime += 1_frame;
        if(laraDeadTime >= core::FrameRate * 10_sec
           || (laraDeadTime >= core::FrameRate * 2_sec && m_presenter->getInputHandler().hasAnyAction()))
        {
          menu = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::DeathMode,
                                                     menu::SaveGamePageMode::Restart,
                                                     false,
                                                     world,
                                                     m_presenter->getRenderViewport());
          throttler.reset();
          continue;
        }
      }

      if(world.getCameraController().getMode() != CameraMode::Cinematic
         && m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu))
      {
        menu
          = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::GameMode,
                                                allowSave ? menu::SaveGamePageMode::Save : menu::SaveGamePageMode::Skip,
                                                true,
                                                world,
                                                m_presenter->getRenderViewport());
        throttler.reset();
        continue;
      }

      if(allowSave && m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Save))
      {
        world.save(std::nullopt);
        throttler.reset();
      }
      else if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
      {
        if(getSavegameMeta(std::nullopt).has_value())
        {
          return {RunResult::RequestLoad, std::nullopt};
        }
      }

      if(allAmmoCheat)
        world.getPlayer().getInventory().fillAllAmmo();

      float blackAlpha = 0;
      if(runtime < BlendInDuration)
      {
        runtime += 1_frame;
        blackAlpha = 1 - runtime.cast<float>() / BlendInDuration.cast<float>();
      }

      ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette(), m_presenter->getUiViewport()};

      drawAmmoWidget(ui, getPresenter().getTrFont(), world, ammoDisplayDuration);
      if(bugReportSavedDuration != 0_frame)
      {
        drawBugReportMessage(ui, getPresenter().getTrFont());
        bugReportSavedDuration -= 1_frame;
      }

      if(ghostManager.reader != nullptr)
      {
        ghostManager.model->apply(world, ghostManager.reader->read());
        for(const auto& room : world.getRooms())
        {
          if(room.physicalId == ghostManager.model->getRoomId())
          {
            setParent(gsl::not_null{ghostManager.model}, room.node);
          }
        }
      }

      world.getPlayer().timeSpent += 1_frame;
      world.gameLoop(godMode, blackAlpha, ui);

      ghostManager.writer->append(world.getObjectManager().getLara().getGhostFrame());
      world.nextGhostFrame();
    }
    else
    {
      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu) || !world.cinematicLoop())
        return {RunResult::NextLevel, std::nullopt};
    }

    if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
    {
      makeScreenshot();
      throttler.reset();
    }

    if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::BugReport))
    {
      takeBugReport(world);
      bugReportSavedDuration = core::FrameRate * 5_sec;
      throttler.reset();
    }
  }
}

void Engine::makeScreenshot()
{
  auto img = m_presenter->takeScreenshot();
  if(!std::filesystem::is_directory(m_userDataPath / "screenshots"))
    std::filesystem::create_directories(m_userDataPath / "screenshots");

  auto filename = getCurrentHumanReadableTimestamp() + ".png";
  img.savePng(m_userDataPath / "screenshots" / filename);
}

void Engine::takeBugReport(world::World& world)
{
  if(!std::filesystem::is_directory(m_userDataPath / "bugreports"))
  {
    std::filesystem::create_directory(m_userDataPath / "bugreports");
  }

  const auto dirName = getCurrentHumanReadableTimestamp();
  if(!std::filesystem::is_directory(m_userDataPath / "bugreports" / dirName))
  {
    std::filesystem::create_directory(m_userDataPath / "bugreports" / dirName);
  }

  std::filesystem::copy_file(m_userDataPath / "croftengine.log",
                             m_userDataPath / "bugreports" / dirName / "croftengine.log");

  auto img = m_presenter->takeScreenshot();
  img.savePng(m_userDataPath / "bugreports" / dirName / "screenshot.png");

  world.save(m_userDataPath / "bugreports" / dirName / "save.yaml", false);
}

std::pair<RunResult, std::optional<size_t>> Engine::runTitleMenu(world::World& world)
{
  gl::Framebuffer::unbindAll();

  applySettings();

  for(const auto& streamInfo : world.getAudioEngine().getStreams())
  {
    if(const auto stream = streamInfo.second.stream.lock())
      stream->setLooping(true);
  }

  const auto backdrop = gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(
                      getAssetDataPath() / std::filesystem::path{m_scriptEngine.getGameflow().getTitleMenuBackdrop()})}
      .toTexture("title"),
    gsl::make_unique<gl::Sampler>("title-sampler"));
  const auto menu = std::make_shared<menu::MenuDisplay>(
    menu::InventoryMode::TitleMode, menu::SaveGamePageMode::NewGame, false, world, m_presenter->getRenderViewport());
  Throttler throttler;
  while(true)
  {
    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitApp, std::nullopt};
    }

    throttler.wait();

    if(!m_presenter->preFrame())
      continue;

    ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette(), m_presenter->getUiViewport()};

    std::shared_ptr<render::scene::Mesh> backdropMesh;
    {
      const auto viewport = glm::vec2{m_presenter->getRenderViewport()};
      const auto sourceSize = glm::vec2{backdrop->getTexture()->size()};
      const float splashScale = std::min(viewport.x / sourceSize.x, viewport.y / sourceSize.y);

      auto scaledSourceSize = sourceSize * splashScale;
      auto sourceOffset = (viewport - scaledSourceSize) / 2.0f;
      backdropMesh = render::scene::createScreenQuad(
        sourceOffset, scaledSourceSize, m_presenter->getMaterialManager()->getBackdrop(false), "backdrop");
      backdropMesh->bind(
        "u_input",
        [backdrop](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
        {
          uniform.set(backdrop);
        });
    }

    backdropMesh->bind(
      "u_input",
      [backdrop](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      {
        uniform.set(backdrop);
      });
    {
      render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
      backdropMesh->render(nullptr, context);
    }
    menu->display(ui, world);
    m_presenter->renderUi(ui, 1);
    m_presenter->updateSoundEngine();
    m_presenter->swapBuffers();
    switch(menu->result)
    {
    case menu::MenuResult::None:
      break;
    case menu::MenuResult::Closed:
      return {RunResult::ExitApp, std::nullopt};
    case menu::MenuResult::ExitToTitle:
      return {RunResult::TitleLevel, std::nullopt};
    case menu::MenuResult::ExitGame:
      return {RunResult::ExitApp, std::nullopt};
    case menu::MenuResult::NewGame:
      return {RunResult::NextLevel, std::nullopt};
    case menu::MenuResult::LaraHome:
      return {RunResult::LaraHomeLevel, std::nullopt};
    case menu::MenuResult::RestartLevel:
      return {RunResult::RestartLevel, std::nullopt};
    case menu::MenuResult::RequestLoad:
      if(getSavegameMeta(menu->requestLoad).has_value())
      {
        return {RunResult::RequestLoad, menu->requestLoad};
      }
    }

    if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
    {
      makeScreenshot();
    }
    else if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
    {
      if(getSavegameMeta(std::nullopt).has_value())
        return {RunResult::RequestLoad, std::nullopt};
    }
  }
}

std::pair<RunResult, std::optional<size_t>>
  Engine::runLevelSequenceItem(script::LevelSequenceItem& item,
                               const std::shared_ptr<Player>& player,
                               const std::shared_ptr<Player>& levelStartPlayer)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  applySettings();
  return item.run(*this, player, levelStartPlayer);
}

std::pair<RunResult, std::optional<size_t>>
  Engine::runLevelSequenceItemFromSave(script::LevelSequenceItem& item,
                                       const std::optional<size_t>& slot,
                                       const std::shared_ptr<Player>& player,
                                       const std::shared_ptr<Player>& levelStartPlayer)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  applySettings();
  return item.runFromSave(*this, slot, player, levelStartPlayer);
}

std::unique_ptr<loader::trx::Glidos> Engine::loadGlidosPack() const
{
  if(m_engineConfig->renderSettings.glidosPack.has_value())
  {
    if(!std::filesystem::is_directory(m_engineConfig->renderSettings.glidosPack.value()))
      return nullptr;

    m_presenter->drawLoadingScreen(_("Loading Glidos texture pack"));
    auto lastUpdate = std::chrono::high_resolution_clock::now();
    static constexpr auto TimePerFrame
      = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::seconds{1})
        / core::FrameRate.get();
    return std::make_unique<loader::trx::Glidos>(m_engineConfig->renderSettings.glidosPack.value(),
                                                 [this, &lastUpdate](const std::string& s)
                                                 {
                                                   const auto now = std::chrono::high_resolution_clock::now();
                                                   if(lastUpdate + TimePerFrame < now)
                                                   {
                                                     lastUpdate = now;
                                                     m_presenter->drawLoadingScreen(s);
                                                   }
                                                 });
  }

  return nullptr;
}

std::optional<SavegameMeta> Engine::getSavegameMeta(const std::filesystem::path& filename) const
{
  std::filesystem::path filepath{getSavegameRootPath() / filename};
  if(!std::filesystem::is_regular_file(filepath))
    return std::nullopt;

  serialization::YAMLDocument<true> doc{filepath};
  SavegameMeta meta{};
  doc.load("meta", meta, meta);
  return meta;
}

std::optional<SavegameMeta> Engine::getSavegameMeta(const std::optional<size_t>& slot) const
{
  return getSavegameMeta(getSavegamePath(slot));
}

void Engine::applySettings()
{
  m_presenter->apply(m_engineConfig->renderSettings, m_engineConfig->audioSettings);
  for(const auto& world : m_worlds)
  {
    world->getAudioEngine().setMusicGain(m_engineConfig->audioSettings.musicVolume);
    world->getAudioEngine().setSfxGain(m_engineConfig->audioSettings.sfxVolume);
    for(auto& room : world->getRooms())
    {
      room.collectShaderLights(m_engineConfig->renderSettings.getLightCollectionDepth());
      gsl_Assert(room.dust != nullptr);
      room.dust->setVisible(m_engineConfig->renderSettings.dust);
    }
  }
}

std::filesystem::path Engine::getSavegameRootPath() const
{
  auto p = m_userDataPath / "saves" / m_gameflowId;
  std::filesystem::create_directories(p);
  return p;
}

std::filesystem::path Engine::getSavegamePath(const std::optional<size_t>& slot) const
{
  const auto root = getSavegameRootPath();
  if(slot.has_value())
    return root / makeSavegameFilename(*slot);
  else
    return root / QuicksaveFilename;
}

std::filesystem::path Engine::getAssetDataPath() const
{
  return m_userDataPath / "data" / m_scriptEngine.getGameflow().getAssetRoot();
}

void SavegameMeta::serialize(const serialization::Serializer<SavegameMeta>& ser)
{
  ser(S_NV("filename", filename), S_NV("title", title));
}
} // namespace engine
