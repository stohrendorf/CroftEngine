#include "engine.h"

#include "audio/soundengine.h"
#include "audio/streamvoice.h"
#include "core/i18n.h"
#include "core/units.h"
#include "engine/audioengine.h"
#include "engine/audiosettings.h"
#include "engine/cameracontroller.h"
#include "engine/engineconfig.h"
#include "engine/ghosting/ghostmodel.h"
#include "engine/inventory.h"
#include "engine/objectmanager.h"
#include "engine/objects/objectstate.h"
#include "engine/world/room.h"
#include "ghosting/ghost.h"
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
#include "render/scene/rendercontext.h"
#include "render/scene/rendermode.h"
#include "script/reflection.h"
#include "script/scriptengine.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "ui/core.h"
#include "ui/levelstats.h"
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
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <gslu.h>
#include <iosfwd>
#include <locale>
#include <pybind11/eval.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace render::scene
{
class Node;
}

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
      ui.drawBox(pos + glm::ivec2{-margin, margin},
                 {text.getWidth() + 2 * margin, -ui::FontHeight - 2 * margin - 2},
                 gl::SRGBA8{0, 0, 0, glm::mix(128, 224, bias)});
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

    ui::Ui ui{presenter->getMaterialManager()->getUi(), world.getPalette(), presenter->getRenderViewport()};
    ui::LevelStats stats{world.getTitle(), world.getTotalSecrets(), world.getPlayerPtr(), presenter};
    stats.draw(ui);

    presenter->renderWorld(world.getRooms(), world.getCameraController(), world.getCameraController().update());
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

struct GhostManager
{
  GhostManager(const std::filesystem::path& recordingPath, world::World& world)
      : writerPath{recordingPath}
      , writer{std::make_unique<ghosting::GhostDataWriter>(recordingPath)}
      , readerPath{std::filesystem::path{recordingPath}.replace_extension(".bin")}
  {
    if(std::filesystem::is_regular_file(readerPath))
    {
      reader = std::make_unique<ghosting::GhostDataReader>(readerPath);
      for(auto i = 0_frame; i < world.getGhostFrame(); i += 1_frame)
      {
        writer->append(reader->read());
      }
    }
    else
    {
      for(auto i = 0_frame; i < world.getGhostFrame(); i += 1_frame)
      {
        writer->append({});
      }
    }
  }

  ~GhostManager()
  {
    writer.reset();
    std::error_code ec;
    std::filesystem::remove(writerPath, ec);
  }

  bool askGhostSave(Presenter& presenter, world::World& world)
  {
    const auto msgBox = std::make_shared<ui::widgets::MessageBox>(
      /* translators: TR charmap encoding */ _("Save recorded ghost?"));
    msgBox->fitToContent();
    msgBox->setConfirmed(false);

    Throttler throttler;
    while(true)
    {
      throttler.wait();

      if(presenter.shouldClose())
      {
        return false;
      }

      if(!presenter.preFrame())
      {
        continue;
      }

      if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Left)
         || presenter.getInputHandler().hasDebouncedAction(hid::Action::Right))
      {
        msgBox->setConfirmed(!msgBox->isConfirmed());
      }

      ui::Ui ui{presenter.getMaterialManager()->getUi(), world.getPalette(), presenter.getRenderViewport()};

      msgBox->setPosition({(ui.getSize().x - msgBox->getSize().x) / 2, (ui.getSize().y - msgBox->getSize().y) / 2});
      msgBox->update(true);
      msgBox->draw(ui, presenter);
      presenter.renderWorld(world.getRooms(), world.getCameraController(), world.getCameraController().update());
      presenter.renderScreenOverlay();
      presenter.renderUi(ui, 1.0f);
      presenter.updateSoundEngine();
      presenter.swapBuffers();
      if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Action))
      {
        if(msgBox->isConfirmed())
        {
          reader.reset();
          writer.reset();

          std::error_code ec;
          std::filesystem::remove(readerPath, ec);

          std::filesystem::rename(writerPath, readerPath, ec);
        }
        return true;
      }
    }
  }

  std::shared_ptr<ghosting::GhostModel> model = std::make_shared<ghosting::GhostModel>();
  const std::filesystem::path readerPath;
  std::unique_ptr<ghosting::GhostDataReader> reader;
  const std::filesystem::path writerPath;
  std::unique_ptr<ghosting::GhostDataWriter> writer;
};
} // namespace

Engine::Engine(std::filesystem::path userDataPath,
               const std::filesystem::path& engineDataPath,
               const glm::ivec2& resolution)
    : m_userDataPath{std::move(userDataPath)}
    , m_engineDataPath{engineDataPath}
    , m_scriptEngine{engineDataPath}
{
  try
  {
    pybind11::eval_file(util::ensureFileExists(m_engineDataPath / "scripts" / "main.py").string());
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
    m_engineConfig->renderSettings.anisotropyLevel = gsl::narrow<uint32_t>(std::llround(gl::getMaxAnisotropyLevel()));
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
  static constexpr auto BlendInDuration = (core::FrameRate * 2_sec).cast<core::Frame>();
  core::Frame ammoDisplayDuration = 0_frame;

  std::filesystem::create_directories(getUserDataPath() / "ghosts");
  GhostManager ghostManager{getUserDataPath() / "ghosts" / (world.getLevelFilename().stem().replace_extension(".rec")),
                            world};

  while(true)
  {
    ghostManager.model->setVisible(m_engineConfig->displaySettings.ghost);

    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitApp, std::nullopt};
    }

    if(world.levelFinished())
    {
      updateTimeSpent();

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
      updateTimeSpent();
      continue;
    }

    if(menu != nullptr)
    {
      updateTimeSpent();

      m_presenter->renderWorld(world.getRooms(), world.getCameraController(), world.getCameraController().update());
      m_presenter->updateSoundEngine();
      m_presenter->renderScreenOverlay();
      ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette(), m_presenter->getRenderViewport()};
      ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, 224});
      world.drawPerformanceBar(ui, throttler.getAverageWaitRatio());
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
      case menu::MenuResult::LaraHome:
        return {RunResult::LaraHomeLevel, std::nullopt};
      case menu::MenuResult::RequestLoad:
        Expects(menu->requestLoad.has_value());
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
        if(laraDeadTime >= core::FrameRate * 10_sec
           || (laraDeadTime >= core::FrameRate * 2_sec && m_presenter->getInputHandler().hasAnyAction()))
        {
          menu = std::make_shared<menu::MenuDisplay>(
            menu::InventoryMode::DeathMode, world, m_presenter->getRenderViewport());
          menu->allowSave = false;
          throttler.reset();
          continue;
        }
      }

      if(world.getCameraController().getMode() != CameraMode::Cinematic
         && m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu))
      {
        updateTimeSpent();
        menu
          = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::GameMode, world, m_presenter->getRenderViewport());
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

      ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette(), m_presenter->getRenderViewport()};

      drawAmmoWidget(ui, getPresenter().getTrFont(), world, ammoDisplayDuration);

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

      world.gameLoop(godMode, throttler.getAverageWaitRatio(), blackAlpha, ui);

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
      updateTimeSpent();
      makeScreenshot();
      throttler.reset();
    }
  }
}

void Engine::makeScreenshot()
{
  auto img = m_presenter->takeScreenshot();
  if(!std::filesystem::is_directory(m_userDataPath / "screenshots"))
    std::filesystem::create_directories(m_userDataPath / "screenshots");

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
  auto filename = boost::format("%04d-%02d-%02d %02d-%02d-%02d.png") % (localTime->tm_year + 1900)
                  % (localTime->tm_mon + 1) % localTime->tm_mday % localTime->tm_hour % localTime->tm_min
                  % localTime->tm_sec;
  img.savePng(m_userDataPath / "screenshots" / filename.str());
}

std::pair<RunResult, std::optional<size_t>> Engine::runTitleMenu(world::World& world)
{
  gl::Framebuffer::unbindAll();

  applySettings();

  if(const auto stream = world.getAudioEngine().getInterceptStream().lock())
    stream->setLooping(true);

  const auto backdrop = gslu::make_nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(m_userDataPath / "data" / "tr1" / "DATA" / "TITLEH.PCX")}.toTexture("title"),
    gslu::make_nn_unique<gl::Sampler>("title-sampler"));
  const auto menu
    = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::TitleMode, world, m_presenter->getRenderViewport());
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

    ui::Ui ui{m_presenter->getMaterialManager()->getUi(), world.getPalette(), m_presenter->getRenderViewport()};

    std::shared_ptr<render::scene::Mesh> backdropMesh;
    {
      const auto viewport = glm::vec2{m_presenter->getRenderViewport()};
      const auto sourceSize = glm::vec2{backdrop->getTexture()->size()};
      const float splashScale = std::min(viewport.x / sourceSize.x, viewport.y / sourceSize.y);

      auto scaledSourceSize = sourceSize * splashScale;
      auto sourceOffset = (viewport - scaledSourceSize) / 2.0f;
      backdropMesh = render::scene::createScreenQuad(
        sourceOffset, scaledSourceSize, m_presenter->getMaterialManager()->getBackdrop(), "backdrop");
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
    auto lastUpdate = std::chrono::high_resolution_clock::now();
    static constexpr auto TimePerFrame
      = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::seconds{1})
        / core::FrameRate.get();
    return std::make_unique<loader::trx::Glidos>(m_userDataPath / m_engineConfig->renderSettings.glidosPack.value(),
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
  auto p = m_userDataPath / "saves";
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
