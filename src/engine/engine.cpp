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
#include "location.h"
#include "menu/menudisplay.h"
#include "network/hauntedcoopclient.h"
#include "objects/laraobject.h"
#include "player.h"
#include "presenter.h"
#include "qs/qs.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/rendersettings.h"
#include "render/scene/mesh.h"
#include "render/scene/node.h"
#include "render/scene/rendercontext.h"
#include "render/scene/sprite.h"
#include "render/scene/translucency.h"
#include "script/reflection.h"
#include "script/scriptengine.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "soundeffects_tr1.h"
#include "throttler.h"
#include "ui/core.h"
#include "ui/detailedlevelstats.h"
#include "ui/levelstats.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util/datetime.h"
#include "util/helpers.h"
#include "world/world.h"

#include <algorithm>
#include <array>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <gl/cimgwrapper.h>
#include <gl/constants.h>
#include <gl/font.h>
#include <gl/glad_init.h>
#include <gl/image.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <ios>
#include <locale>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
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
    static constexpr auto StaticDuration = core::FrameRate * 1_sec * 2 / 3;
    static constexpr auto TransitionDuration = core::FrameRate * 1_sec / 2;

    ammoDisplayDuration = std::min(ammoDisplayDuration + 1_frame, StaticDuration + TransitionDuration);

    auto drawAmmoText = [&ui, &world, &trFont](float bias)
    {
      const auto& ammo = world.getPlayer().getInventory().getAmmo(world.getPlayer().selectedWeaponType);
      auto text = ui::Text{ui::makeAmmoString(ammo.getDisplayString())};
      const auto margin = ui::FontHeight / 2 + glm::mix(ui::FontHeight, 0, bias);
      const auto targetPos
        = glm::ivec2{ui.getSize().x - ui::FontHeight - 1 - margin - text.getWidth(), ui::FontHeight * 2 + margin};
      const auto center = glm::ivec2{(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y * 30 / 100};
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

void drawSaveReminder(ui::Ui& ui, const ui::TRFont& trFont)
{
  auto text = ui::Text{/* translators: TR charmap encoding */ _("Save Game")};
  const auto pos = glm::ivec2{(ui.getSize().x - text.getWidth()) / 2, ui::FontHeight};
  text.draw(ui, trFont, pos);
}

void makeScreenshot(const Presenter& presenter, const std::filesystem::path& userDataPath)
{
  auto img = presenter.takeScreenshot();
  if(!std::filesystem::is_directory(userDataPath / "screenshots"))
    std::filesystem::create_directories(userDataPath / "screenshots");

  auto filename = util::getCurrentHumanReadableTimestamp() + ".png";
  img.savePng(userDataPath / "screenshots" / filename, false);
}

bool showLevelStats(const std::shared_ptr<Presenter>& presenter,
                    world::World& world,
                    const std::filesystem::path& userDataPath)
{
  static constexpr const auto BlendDuration = 30_frame;
  auto currentBlendDuration = 0_frame;

  const ui::LevelStats stats{world.getTitle(), world.getTotalSecrets(), world.getPlayerPtr(), presenter};
  const ui::DetailedLevelStats detailedStats{world};
  bool detailed = false;

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

    if(presenter->getInputHandler().hasDebouncedAction(hid::Action::SecondaryInteraction))
      detailed = !detailed;

    ui::Ui ui{
      presenter->getMaterialManager()->getUi(), world.getWorldGeometry().getPalette(), presenter->getUiViewport()};
    ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, ui::DefaultBackgroundAlpha});
    if(detailed)
      detailedStats.draw(ui, *presenter, false);
    else
      stats.draw(ui);

    {
      const auto portals = world.getCameraController().update();
      if(const auto lara = world.getObjectManager().getLaraPtr())
        lara->m_state.location.room->node->setVisible(true);
      presenter->renderWorld(world.getRooms(), world.getCameraController(), portals, world);
    }
    presenter->renderScreenOverlay();

    if(currentBlendDuration < BlendDuration)
      currentBlendDuration += 1_frame;

    presenter->renderUi(ui, currentBlendDuration.cast<float>() / BlendDuration.cast<float>());
    presenter->updateSoundEngine();
    presenter->swapBuffers();

    if(presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
    {
      makeScreenshot(*presenter, userDataPath);
      throttler.reset();
    }

    if(presenter->getInputHandler().hasDebouncedAction(hid::Action::Action))
      break;
  }

  return true;
}

void updateGhostRoom(const std::vector<world::Room>& rooms,
                     const gsl::not_null<std::shared_ptr<ghosting::GhostModel>>& ghost)
{
  for(const auto& room : rooms)
  {
    if(room.physicalId != ghost->getRoomId())
      continue;

    setParent(ghost, room.node);

    if(room.node->isVisible())
    {
      // valid room selected
    }
    else if(room.alternateRoom != nullptr && room.alternateRoom->node->isVisible())
    {
      setParent(ghost, room.alternateRoom->node);
    }
    else
    {
      for(const auto& altRoom : rooms)
      {
        if(altRoom.alternateRoom != &room || !altRoom.node->isVisible())
          continue;

        setParent(ghost, altRoom.node);
        break;
      }
    }
    break;
  }
}

void updateRemoteGhosts(world::World& world, GhostManager& ghostManager, const network::HauntedCoopClient& coop)
{
  const auto states = coop.getStates();
  for(const auto& state : states)
  {
    const auto peerId = std::get<0>(state);

    ghosting::GhostFrame ghostFrame;
    std::string tmp;
    const auto& dataVec = std::get<1>(state);
    if(dataVec.empty())
      continue;

    for(const auto v : dataVec)
      tmp += gsl::narrow_cast<char>(v);
    std::istringstream stateDataStream{tmp, std::ios::in | std::ios::binary};
    ghostFrame.read(stateDataStream);

    gsl_Assert(tmp.size() - stateDataStream.tellg() >= 3);
    std::array<uint8_t, 3> ghostColor{};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    stateDataStream.read(reinterpret_cast<char*>(ghostColor.data()), ghostColor.size());
    const auto glColor = gl::SRGB8(ghostColor[0], ghostColor[1], ghostColor[2]);

    auto ghostUsername = network::io::readPascalString(stateDataStream);

    auto it = ghostManager.getRemoteModels().find(peerId);
    if(it == ghostManager.getRemoteModels().end())
    {
      it = ghostManager.getRemoteModels().emplace(peerId, std::make_shared<ghosting::GhostModel>()).first;

      static constexpr const glm::ivec2 nameTextureSize{512, 128};
      static constexpr const int nameFontSize = 48;

      gl::Image<gl::ScalarByte> img{nameTextureSize, nullptr};

      const auto fontMeasurement = world.getPresenter().getGhostNameFont().measure(ghostUsername, nameFontSize);
      BOOST_LOG_TRIVIAL(debug) << "Font measurement x=" << fontMeasurement.first.x << ", y=" << fontMeasurement.first.y
                               << " / x=" << fontMeasurement.second.x << ", y=" << fontMeasurement.second.y;
      world.getPresenter().getGhostNameFont().drawText(
        img,
        ghostUsername,
        {(nameTextureSize.x - fontMeasurement.second.x) / 2, nameTextureSize.y - 1 + fontMeasurement.first.y},
        nameFontSize);

      auto texture = gsl::make_shared<gl::Texture2D<gl::ScalarByte>>(nameTextureSize, 8, "ghost-name");
      texture->assign(img.getData());
      texture->generateMipmaps();
      auto nameHandle = gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::ScalarByte>>>(
        texture,
        gsl::make_unique<gl::Sampler>("ghost-name" + gl::SamplerSuffix) | set(gl::api::TextureMagFilter::Linear)
          | set(gl::api::TextureMinFilter::Linear));

      auto mesh = render::scene::createSpriteMesh(-nameTextureSize.x / 2.0f,
                                                  0,
                                                  nameTextureSize.x / 2.0f,
                                                  gsl::narrow_cast<float>(nameTextureSize.y),
                                                  {0.0f, 1.0f},
                                                  {1.0f, 0.0f},
                                                  render::material::RenderMode::FullNonOpaque,
                                                  world.getPresenter().getMaterialManager()->getGhostName(),
                                                  0,
                                                  "ghost-name");
      auto nameNode = gsl::make_shared<render::scene::Node>("ghost-name");
      nameNode->setRenderable(mesh);
      nameNode->setLocalMatrix(glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, core::LaraWalkHeight.get(), 0.0f}));
      nameNode->bind("u_color",
                     [color = glm::vec3{glColor.channels} / 255.0f](
                       const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     {
                       uniform.set(color);
                     });
      nameNode->bind("u_texture",
                     [nameHandle](const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     {
                       uniform.set(nameHandle);
                     });
      setParent(nameNode, it->second);
    }

    const gsl::not_null remoteGhost{it->second};
    remoteGhost->apply(world, ghostFrame);
    remoteGhost->setColor(glColor);
    updateGhostRoom(world.getRooms(), remoteGhost);
  }

  std::set<uint64_t> ghostsToDrop;
  for(const auto& [id, _] : ghostManager.getRemoteModels())
    ghostsToDrop.emplace(id);
  for(const auto& [id, _] : states)
    ghostsToDrop.erase(id);
  for(const auto& id : ghostsToDrop)
  {
    setParent(gsl::not_null{ghostManager.getRemoteModels().at(id)}, nullptr);
    ghostManager.getRemoteModels().erase(id);
  }
}
} // namespace

Engine::Engine(std::filesystem::path userDataPath,
               const std::filesystem::path& engineDataPath,
               const std::optional<std::string>& localOverride,
               const std::string& gameflowId,
               const glm::ivec2& resolution,
               const bool borderlessFullscreen)
    : m_userDataPath{std::move(userDataPath)}
    , m_engineDataPath{engineDataPath}
    , m_gameflowId{gameflowId}
    , m_scriptEngine{engineDataPath / "gameflows" / gameflowId}
    , m_engineConfig{gsl::make_shared<EngineConfig>()}
{
  {
    const auto invalid = m_scriptEngine.getGameflow().getInvalidFilepaths(getAssetDataPath());
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

  if(std::filesystem::is_regular_file(m_userDataPath / "config.yaml"))
  {
    serialization::YAMLDocument<true> doc{m_userDataPath / "config.yaml"};
    doc.deserialize("config", gsl::not_null{m_engineConfig.get().get()}, *m_engineConfig);
  }

  m_presenter
    = std::make_shared<Presenter>(m_engineDataPath, resolution, m_engineConfig->renderSettings, borderlessFullscreen);
  if(gl::hasAnisotropicFilteringExtension()
     && gsl::narrow_cast<float>(m_engineConfig->renderSettings.anisotropyLevel) > gl::getMaxAnisotropyLevel())
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
  doc.serialize("config", gsl::not_null{m_engineConfig.get().get()}, *m_engineConfig);
  doc.write();
}

std::pair<RunResult, std::optional<size_t>> Engine::run(world::World& world, bool isCutscene, bool allowSave)
{
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
  static constexpr auto BlendInDuration = core::FrameRate * 2_sec;
  core::Frame ammoDisplayDuration = 0_frame;
  core::Frame bugReportSavedDuration = 0_frame;

  const auto ghostRoot = m_userDataPath / "ghosts" / m_gameflowId;
  std::filesystem::create_directories(ghostRoot);
  GhostManager ghostManager{ghostRoot / (world.getLevelFilename().stem().replace_extension(".rec")), world};

  network::HauntedCoopClient coop{world.getEngine().getGameflowId(), world.getLevelFilename().stem().string()};
  coop.start();
  const auto coopStop = gsl::final_action(
    [&coop]()
    {
      coop.stop();
    });

  while(true)
  {
    ghostManager.getModel()->setVisible(m_engineConfig->displaySettings.ghost);
    updateRemoteGhosts(world, ghostManager, coop);
    ghostManager.setChildrenVisibility(m_engineConfig->displaySettings.showCoopNames);

    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitGame, std::nullopt};
    }

    if(world.levelFinished())
    {
      if(!isCutscene && allowSave)
      {
        if(!showLevelStats(m_presenter, world, m_userDataPath))
          return {RunResult::ExitGame, std::nullopt};

        if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
          return {RunResult::ExitGame, std::nullopt};
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
        m_presenter->renderWorld(world.getRooms(), world.getCameraController(), portals, world);
      }
      m_presenter->updateSoundEngine();
      m_presenter->renderScreenOverlay();
      ui::Ui ui{m_presenter->getMaterialManager()->getUi(),
                world.getWorldGeometry().getPalette(),
                m_presenter->getUiViewport()};
      ui.drawBox({0, 0}, ui.getSize(), gl::SRGBA8{0, 0, 0, ui::DefaultBackgroundAlpha});
      m_presenter->renderUi(ui, 1);
      menu->renderObjects(ui, world);
      menu->update(ui, world);
      if(m_presenter->renderSettingsChanged())
      {
        ui.reset();
        continue;
      }

      m_presenter->withBackbuffer(
        [&menu, &world]()
        {
          const render::scene::RenderContext context{
            render::material::RenderMode::FullOpaque, std::nullopt, render::scene::Translucency::Opaque};
          menu->renderRenderedObjects(world);
        });

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
            return {RunResult::ExitGame, std::nullopt};
        }
        m_gameplayRules = {};
        return {RunResult::TitleLevel, std::nullopt};
      case menu::MenuResult::ExitGame:
        if(allowSave)
        {
          if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
            return {RunResult::ExitGame, std::nullopt};
        }
        return {RunResult::ExitGame, std::nullopt};
      case menu::MenuResult::NewGame:
        if(allowSave)
        {
          if(m_engineConfig->displaySettings.ghost && !ghostManager.askGhostSave(*m_presenter, world))
            return {RunResult::ExitGame, std::nullopt};
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
              return {RunResult::ExitGame, std::nullopt};
          }
          return {RunResult::RequestLoad, menu->requestLoad};
        }
        break;
      case menu::MenuResult::RequestLevel:
        return {RunResult::RequestLevel, menu->requestLevelSequenceIndex};
      }

      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
      {
        if(m_gameplayRules.noLoads)
        {
          if(auto lara = world.getObjectManager().getLaraPtr(); lara != nullptr)
          {
            lara->playSoundEffect(TR1SoundEffect::LaraNo);
          }
        }
        else if(getSavegameMeta(std::nullopt).has_value())
        {
          return {RunResult::RequestLoad, std::nullopt};
        }
      }
      continue;
    }

    if(isCutscene)
    {
      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu) || !world.cinematicLoop())
        return {RunResult::NextLevel, std::nullopt};
    }
    else
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
        if(m_gameplayRules.noLoads)
        {
          if(auto lara = world.getObjectManager().getLaraPtr(); lara != nullptr)
          {
            lara->playSoundEffect(TR1SoundEffect::LaraNo);
          }
        }
        else if(getSavegameMeta(std::nullopt).has_value())
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

      ui::Ui ui{m_presenter->getMaterialManager()->getUi(),
                world.getWorldGeometry().getPalette(),
                m_presenter->getUiViewport()};

      drawAmmoWidget(ui, getPresenter().getTrFont(), world, ammoDisplayDuration);
      if(bugReportSavedDuration != 0_frame)
      {
        drawBugReportMessage(ui, getPresenter().getTrFont());
        bugReportSavedDuration -= 1_frame;
      }

      if(allowSave && m_engineConfig->saveReminderEnabled && std::chrono::steady_clock::now() >= m_saveReminderSince)
      {
        drawSaveReminder(ui, getPresenter().getTrFont());
      }

      if(ghostManager.getReader() != nullptr)
      {
        ghostManager.getModel()->apply(world, ghostManager.getReader()->read());
        updateGhostRoom(world.getRooms(), gsl::not_null{ghostManager.getModel()});
      }

      world.getPlayer().timeSpent += 1_frame;
      world.gameLoop(godMode, blackAlpha, ui);

      const auto frame = world.getObjectManager().getLara().getGhostFrame();

      std::ostringstream stateDataStream{std::ios::out | std::ios::binary};
      frame.write(stateDataStream);
      std::vector<uint8_t> stateData;
      for(const auto c : stateDataStream.str())
        stateData.emplace_back(c);
      coop.sendState(stateData);
      ghostManager.getWriter()->append(frame);
      world.nextGhostFrame();
    }

    if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
    {
      makeScreenshot(*m_presenter, m_userDataPath);
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

void Engine::takeBugReport(world::World& world)
{
  if(!std::filesystem::is_directory(m_userDataPath / "bugreports"))
  {
    std::filesystem::create_directory(m_userDataPath / "bugreports");
  }

  const auto dirName = util::getCurrentHumanReadableTimestamp();
  if(!std::filesystem::is_directory(m_userDataPath / "bugreports" / dirName))
  {
    std::filesystem::create_directory(m_userDataPath / "bugreports" / dirName);
  }

  for(std::filesystem::directory_iterator it{m_userDataPath}; it != std::filesystem::directory_iterator{}; ++it)
  {
    BOOST_LOG_TRIVIAL(info) << it->path() << "; " << it->path().filename().string();
    if(!it->is_regular_file() || !boost::algorithm::starts_with(it->path().filename().string(), "croftengine.")
       || !boost::algorithm::ends_with(it->path().filename().string(), ".log"))
      continue;

    std::filesystem::copy_file(it->path(), m_userDataPath / "bugreports" / dirName / it->path().filename());
  }

  auto img = m_presenter->takeScreenshot();
  img.savePng(m_userDataPath / "bugreports" / dirName / "screenshot.png", false);

  world.save(m_userDataPath / "bugreports" / dirName / "save.yaml");
}

std::pair<RunResult, std::optional<size_t>> Engine::runTitleMenu(world::World& world)
{
  applySettings();

  for(const auto& [slot, slotStream] : world.getAudioEngine().getSoundEngine().getSlots())
  {
    if(const auto stream = slotStream.stream.lock())
      stream->setLooping(true);
  }

  const auto backdrop = gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(
                      getAssetDataPath() / std::filesystem::path{m_scriptEngine.getGameflow().getTitleMenuBackdrop()})}
      .toTexture("title"),
    gsl::make_unique<gl::Sampler>("title" + gl::SamplerSuffix));
  const auto menu = std::make_shared<menu::MenuDisplay>(
    menu::InventoryMode::TitleMode, menu::SaveGamePageMode::NewGame, false, world, m_presenter->getRenderViewport());
  Throttler throttler;
  while(true)
  {
    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitGame, std::nullopt};
    }

    throttler.wait();

    if(!m_presenter->preFrame())
      continue;

    ui::Ui ui{
      m_presenter->getMaterialManager()->getUi(), world.getWorldGeometry().getPalette(), m_presenter->getUiViewport()};

    std::shared_ptr<render::scene::Mesh> backdropMesh;
    {
      const auto viewport = glm::vec2{m_presenter->getRenderViewport()};
      const auto sourceSize = glm::vec2{backdrop->getTexture()->size()};
      const float splashScale = std::min(viewport.x / sourceSize.x, viewport.y / sourceSize.y);

      auto scaledSourceSize = sourceSize * splashScale;
      auto sourceOffset = (viewport - scaledSourceSize) / 2.0f;
      backdropMesh = render::scene::createScreenQuad(sourceOffset,
                                                     scaledSourceSize,
                                                     m_presenter->getMaterialManager()->getBackdrop(false),
                                                     render::scene::Translucency::Opaque,
                                                     "backdrop");
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

    menu->renderObjects(ui, world);
    menu->update(ui, world);
    m_presenter->withBackbuffer(
      [&backdropMesh, &menu, &world]()
      {
        {
          render::scene::RenderContext context{
            render::material::RenderMode::FullOpaque, std::nullopt, render::scene::Translucency::Opaque};
          backdropMesh->render(nullptr, context);
          menu->renderRenderedObjects(world);
        }
      });
    if(m_presenter->renderSettingsChanged())
    {
      continue;
    }

    m_presenter->renderUi(ui, 1);
    m_presenter->updateSoundEngine();
    m_presenter->swapBuffers();
    switch(menu->result)
    {
    case menu::MenuResult::None:
      break;
    case menu::MenuResult::Closed:
      return {RunResult::ExitGame, std::nullopt};
    case menu::MenuResult::ExitToTitle:
      return {RunResult::TitleLevel, std::nullopt};
    case menu::MenuResult::ExitGame:
      return {RunResult::ExitGame, std::nullopt};
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
      break;
    case menu::MenuResult::RequestLevel:
      return {RunResult::RequestLevel, menu->requestLevelSequenceIndex};
    }

    if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Screenshot))
    {
      makeScreenshot(*m_presenter, m_userDataPath);
    }
    else if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
    {
      if(getSavegameMeta(std::nullopt).has_value())
        return {RunResult::RequestLoad, std::nullopt};
    }
  } // namespace engine
}

std::pair<RunResult, std::optional<size_t>>
  Engine::runLevelSequenceItem(script::LevelSequenceItem& item,
                               const std::shared_ptr<Player>& player,
                               const std::shared_ptr<Player>& levelStartPlayer)
{
  m_presenter->clear();
  applySettings();
  return item.run(gsl::not_null{this}, player, levelStartPlayer);
}

std::pair<RunResult, std::optional<size_t>>
  Engine::runLevelSequenceItemFromSave(script::LevelSequenceItem& item,
                                       const std::optional<size_t>& slot,
                                       const std::shared_ptr<Player>& player,
                                       const std::shared_ptr<Player>& levelStartPlayer)
{
  m_presenter->clear();
  applySettings();
  return item.runFromSave(gsl::not_null{this}, slot, player, levelStartPlayer);
}

std::unique_ptr<loader::trx::Glidos> Engine::loadGlidosPack() const
{
  if(!m_engineConfig->renderSettings.glidosPack.has_value())
    return nullptr;

  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  if(!std::filesystem::is_directory(m_engineConfig->renderSettings.glidosPack.value()))
    return nullptr;

  m_presenter->drawLoadingScreen(_("Loading Glidos texture pack"));
  auto lastUpdate = std::chrono::high_resolution_clock::now();
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  return std::make_unique<loader::trx::Glidos>(m_engineConfig->renderSettings.glidosPack.value(),
                                               [this, &lastUpdate](const std::string& s)
                                               {
                                                 const auto now = std::chrono::high_resolution_clock::now();
                                                 if(lastUpdate + core::TimePerFrame < now)
                                                 {
                                                   lastUpdate = now;
                                                   m_presenter->drawLoadingScreen(s);
                                                 }
                                               });
}

std::optional<SavegameMeta> Engine::getSavegameMeta(const std::filesystem::path& filename) const
{
  const std::filesystem::path filepath{getSavegameRootPath() / filename};
  if(!std::filesystem::is_regular_file(filepath))
    return std::nullopt;

  serialization::YAMLDocument<true> doc{filepath};
  SavegameMeta meta{};
  doc.deserialize("meta", gsl::not_null{&meta}, meta);
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
      room.regenerateDust(*m_presenter,
                          m_presenter->getMaterialManager()->getDustParticle(),
                          m_engineConfig->renderSettings.dustActive,
                          m_engineConfig->renderSettings.dustDensity);
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

void Engine::onGameSavedOrLoaded()
{
  m_saveReminderSince = std::chrono::steady_clock::now() + std::chrono::minutes{m_engineConfig->saveReminderMinutes};
}

void SavegameMeta::serialize(const serialization::Serializer<SavegameMeta>& ser) const
{
  ser(S_NV("filename", filename));
}

void SavegameMeta::deserialize(const serialization::Deserializer<SavegameMeta>& ser)
{
  ser(S_NV("filename", filename));
}
} // namespace engine
