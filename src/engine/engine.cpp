#include "engine.h"

#include "audio/soundengine.h"
#include "audio/streamvoice.h"
#include "audio/tracktype.h"
#include "core/i18n.h"
#include "engine/ai/ai.h"
#include "engine/audioengine.h"
#include "floordata/floordata.h"
#include "hid/inputhandler.h"
#include "loader/file/level/level.h"
#include "loader/trx/trx.h"
#include "menu/menudisplay.h"
#include "objects/aiagent.h"
#include "objects/block.h"
#include "objects/laraobject.h"
#include "objects/modelobject.h"
#include "objects/objectfactory.h"
#include "objects/pickupobject.h"
#include "objects/tallblock.h"
#include "player.h"
#include "presenter.h"
#include "render/renderpipeline.h"
#include "render/scene/csm.h"
#include "render/scene/materialmanager.h"
#include "render/scene/renderer.h"
#include "render/scene/screenoverlay.h"
#include "render/textureanimator.h"
#include "script/reflection.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "tracks_tr1.h"
#include "ui/levelstats.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "world/rendermeshdata.h"
#include "world/world.h"

#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/range/adaptor/map.hpp>
#include <cstdlib>
#include <filesystem>
#include <gl/font.h>
#include <gl/glew_init.h>
#include <gl/texture2d.h>
#include <glm/gtx/norm.hpp>
#include <locale>
#include <numeric>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace engine
{
namespace
{
const gsl::czstring QuicksaveFilename = "quicksave.yaml";
} // namespace

Engine::Engine(const std::filesystem::path& rootPath, const glm::ivec2& resolution)
    : m_rootPath{rootPath}
    , m_scriptEngine{rootPath}
{
  try
  {
    pybind11::eval_file(util::ensureFileExists(m_rootPath / "scripts" / "main.py").string());
  }
  catch(std::exception& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.py: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load main.py"));
  }

  m_locale = std::use_facet<boost::locale::info>(boost::locale::generator()("")).name();
  BOOST_LOG_TRIVIAL(info) << "Detected user's locale is " << m_locale;
  if(const auto overrideLocale = m_scriptEngine.getLocaleOverride())
  {
    m_locale = *overrideLocale;
    BOOST_LOG_TRIVIAL(info) << "Locale override is " << m_locale;
  }

  core::setLocale(std::filesystem::absolute(std::filesystem::current_path() / "share" / "po"), m_locale);

  m_engineConfig = std::make_unique<EngineConfig>();
  if(std::filesystem::is_regular_file(m_rootPath / "config.yaml"))
  {
    serialization::YAMLDocument<true> doc{m_rootPath / "config.yaml"};
    doc.load("config", *m_engineConfig, *m_engineConfig);
  }

  m_presenter = std::make_shared<Presenter>(m_rootPath, resolution);
  if(gl::hasAnisotropicFilteringExtension()
     && m_engineConfig->renderSettings.anisotropyLevel > gl::getMaxAnisotropyLevel())
    m_engineConfig->renderSettings.anisotropyLevel = gsl::narrow<uint32_t>(std::llround(gl::getMaxAnisotropyLevel()));
  applySettings();
  m_presenter->getInputHandler().setMappings(m_engineConfig->inputMappings);
  m_glidos = loadGlidosPack();
}

Engine::~Engine()
{
  serialization::YAMLDocument<false> doc{m_rootPath / "config.yaml"};
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

  const bool godMode = m_scriptEngine.isGodMode();
  const bool allAmmoCheat = m_scriptEngine.hasAllAmmoCheat();

  applySettings();
  std::shared_ptr<menu::MenuDisplay> menu;
  Throttler throttler;
  core::Frame laraDeadTime = 0_frame;

  std::optional<std::chrono::high_resolution_clock::time_point> gameSessionStart;
  auto updateTimeSpent = [&gameSessionStart, &world]()
  {
    if(gameSessionStart.has_value())
    {
      world.getPlayer().timeSpent += std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - gameSessionStart.value());
      gameSessionStart.reset();
    }
  };

  core::Frame runtime = 0_frame;
  static constexpr core::Frame BlendInDuration = 60_frame;
  while(true)
  {
    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitApp, std::nullopt};
    }

    if(world.levelFinished())
    {
      updateTimeSpent();

      if(!isCutscene && allowSave)
      {
        static constexpr const auto BlendDuration = 30_frame;
        auto currentBlendDuration = 0_frame;

        while(true)
        {
          throttler.wait();
          if(m_presenter->shouldClose())
          {
            return {RunResult::ExitApp, std::nullopt};
          }

          if(!m_presenter->preFrame())
          {
            continue;
          }

          ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette()};
          ui::LevelStats stats{world.getTitle(), world.getTotalSecrets(), world.getPlayerPtr(), m_presenter};
          stats.draw(ui);

          m_presenter->renderWorld(world.getObjectManager(),
                                   world.getRooms(),
                                   world.getCameraController(),
                                   world.getCameraController().update(),
                                   throttler.getAverageDelayRatio());
          m_presenter->renderScreenOverlay();

          if(currentBlendDuration < BlendDuration)
            currentBlendDuration += 1_frame;

          m_presenter->renderUi(ui, currentBlendDuration.cast<float>() / BlendDuration.cast<float>());
          m_presenter->updateSoundEngine();
          m_presenter->swapBuffers();

          if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Action))
            break;
        }
      }

      return {RunResult::NextLevel, std::nullopt};
    }

    throttler.wait();
    if(!m_presenter->preFrame())
    {
      updateTimeSpent();
      continue;
    }

    if(menu != nullptr)
    {
      updateTimeSpent();

      render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
      m_presenter->renderWorld(world.getObjectManager(),
                               world.getRooms(),
                               world.getCameraController(),
                               world.getCameraController().update(),
                               throttler.getAverageDelayRatio());
      m_presenter->updateSoundEngine();
      m_presenter->renderScreenOverlay();
      ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette()};
      ui.drawBox({0, 0}, m_presenter->getViewport(), gl::SRGBA8{0, 0, 0, 224});
      world.drawPerformanceBar(ui, throttler.getAverageDelayRatio());
      m_presenter->renderUi(ui, 1);
      menu->display(ui, world);
      m_presenter->renderUi(ui, 1);
      m_presenter->swapBuffers();
      switch(menu->result)
      {
      case menu::MenuResult::None: break;
      case menu::MenuResult::Closed:
        menu.reset();
        throttler.reset();
        break;
      case menu::MenuResult::ExitToTitle: return {RunResult::TitleLevel, std::nullopt};
      case menu::MenuResult::ExitGame: return {RunResult::ExitApp, std::nullopt};
      case menu::MenuResult::NewGame: return {RunResult::NextLevel, std::nullopt};
      case menu::MenuResult::LaraHome: return {RunResult::LaraHomeLevel, std::nullopt};
      case menu::MenuResult::RequestLoad:
        Expects(menu->requestLoad.has_value());
        if(getSavegameMeta(menu->requestLoad).has_value())
        {
          return {RunResult::RequestLoad, menu->requestLoad};
        }
      }

      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
      {
        if(getSavegameMeta(std::nullopt).has_value())
        {
          updateTimeSpent();
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
        updateTimeSpent();
        laraDeadTime += 1_frame;
        if(laraDeadTime >= 300_frame || (laraDeadTime >= 60_frame && m_presenter->getInputHandler().hasAnyAction()))
        {
          menu = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::DeathMode, world);
          menu->allowSave = false;
          throttler.reset();
          continue;
        }
      }

      if(world.getCameraController().getMode() != CameraMode::Cinematic
         && m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu))
      {
        updateTimeSpent();
        menu = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::GameMode, world);
        menu->allowSave = allowSave;
        throttler.reset();
        continue;
      }

      if(allowSave && m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Save))
      {
        updateTimeSpent();
        world.save(std::nullopt);
        throttler.reset();
      }
      else if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
      {
        if(getSavegameMeta(std::nullopt).has_value())
        {
          updateTimeSpent();
          return {RunResult::RequestLoad, std::nullopt};
        }
      }

      if(allAmmoCheat)
        world.getPlayer().getInventory().fillAllAmmo();

      if(!gameSessionStart.has_value())
        gameSessionStart = std::chrono::high_resolution_clock::now();

      float blackAlpha = 0;
      if(runtime < BlendInDuration)
      {
        runtime += 1_frame;
        blackAlpha = 1 - runtime.cast<float>() / BlendInDuration.cast<float>();
      }
      world.gameLoop(godMode, throttler.getAverageDelayRatio(), blackAlpha);
    }
    else
    {
      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu) || !world.cinematicLoop())
        return {RunResult::NextLevel, std::nullopt};
    }

    if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
    {
      updateTimeSpent();
      makeScreenshot();
      throttler.reset();
    }
  }
}

void Engine::makeScreenshot()
{
  auto img = m_presenter->takeScreenshot();
  if(!std::filesystem::is_directory(m_rootPath / "screenshots"))
    std::filesystem::create_directories(m_rootPath / "screenshots");

  auto time = std::time(nullptr);
#ifdef WIN32
  struct tm localTimeData
  {
  };
  Expects(localtime_s(&localTimeData, &time) == 0);
  auto localTime = &localTimeData;
#else
  auto localTime = std::localtime(&time);
#endif
  auto filename = boost::format("%04d-%02d-%02d %02d-%02d-%02d.png") % (localTime->tm_year + 1900)
                  % (localTime->tm_mon + 1) % localTime->tm_mday % localTime->tm_hour % localTime->tm_min
                  % localTime->tm_sec;
  img.savePng(m_rootPath / "screenshots" / filename.str());
}

std::pair<RunResult, std::optional<size_t>> Engine::runTitleMenu(world::World& world)
{
  gl::Framebuffer::unbindAll();

  applySettings();

  Expects(world.getAudioEngine().getInterceptStream() != nullptr);
  world.getAudioEngine().getInterceptStream()->setLooping(true);

  const auto backdrop = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(m_rootPath / "data" / "tr1" / "DATA" / "TITLEH.PCX")}.toTexture());
  const auto menu = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::TitleMode, world);
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

    ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette()};
    render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};

    std::shared_ptr<render::scene::Mesh> backdropMesh;
    {
      const auto targetSize = glm::vec2{m_presenter->getViewport()};
      const auto sourceSize = glm::vec2{backdrop->getTexture()->size()};
      const float splashScale = std::min(targetSize.x / sourceSize.x, targetSize.y / sourceSize.y);

      auto scaledSourceSize = sourceSize * splashScale;
      auto sourceOffset = (targetSize - scaledSourceSize) / 2.0f;
      backdropMesh = render::scene::createScreenQuad(
        sourceOffset, scaledSourceSize, m_presenter->getMaterialManager()->getBackdrop(), "backdrop");
      backdropMesh->bind(
        "u_input",
        [backdrop](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
        { uniform.set(backdrop); });
      backdropMesh->bind(
        "u_screenSize",
        [targetSize](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
        { uniform.set(targetSize); });
    }

    backdropMesh->bind(
      "u_input",
      [backdrop](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      { uniform.set(backdrop); });
    backdropMesh->bind(
      "u_screenSize",
      [this](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
      { uniform.set(glm::vec2{m_presenter->getViewport()}); });
    backdropMesh->render(context);
    menu->display(ui, world);
    m_presenter->renderUi(ui, 1);
    m_presenter->updateSoundEngine();
    m_presenter->swapBuffers();
    switch(menu->result)
    {
    case menu::MenuResult::None: break;
    case menu::MenuResult::Closed: return {RunResult::ExitApp, std::nullopt};
    case menu::MenuResult::ExitToTitle: return {RunResult::TitleLevel, std::nullopt};
    case menu::MenuResult::ExitGame: return {RunResult::ExitApp, std::nullopt};
    case menu::MenuResult::NewGame: return {RunResult::NextLevel, std::nullopt};
    case menu::MenuResult::LaraHome: return {RunResult::LaraHomeLevel, std::nullopt};
    case menu::MenuResult::RequestLoad:
      Expects(menu->requestLoad.has_value());
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

std::pair<RunResult, std::optional<size_t>> Engine::runLevelSequenceItem(script::LevelSequenceItem& item,
                                                                         const std::shared_ptr<Player>& player)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  applySettings();
  return item.run(*this, player);
}

std::pair<RunResult, std::optional<size_t>> Engine::runLevelSequenceItemFromSave(script::LevelSequenceItem& item,
                                                                                 const std::optional<size_t>& slot,
                                                                                 const std::shared_ptr<Player>& player)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  applySettings();
  return item.runFromSave(*this, slot, player);
}

std::unique_ptr<loader::trx::Glidos> Engine::loadGlidosPack() const
{
  if(m_engineConfig->renderSettings.glidosPack.has_value())
  {
    if(!std::filesystem::is_directory(m_engineConfig->renderSettings.glidosPack.value()))
      return nullptr;

    m_presenter->drawLoadingScreen(_("Loading Glidos texture pack"));
    return std::make_unique<loader::trx::Glidos>(m_rootPath / m_engineConfig->renderSettings.glidosPack.value(),
                                                 [this](const std::string& s) { m_presenter->drawLoadingScreen(s); });
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
      room.collectShaderLights(m_engineConfig->renderSettings.getLightCollectionDepth());
  }
}

std::filesystem::path Engine::getSavegameRootPath() const
{
  auto p = m_rootPath / "saves";
  if(!std::filesystem::is_directory(p))
    std::filesystem::create_directory(p);
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

void SavegameMeta::serialize(const serialization::Serializer<SavegameMeta>& ser)
{
  ser(S_NV("filename", filename), S_NV("title", title));
}
} // namespace engine
