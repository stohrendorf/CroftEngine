#include "engine.h"

#include "audio/tracktype.h"
#include "core/pybindmodule.h"
#include "engine/ai/ai.h"
#include "engine/audioengine.h"
#include "floordata/floordata.h"
#include "hid/inputhandler.h"
#include "i18n.h"
#include "loader/file/level/level.h"
#include "loader/file/rendermeshdata.h"
#include "loader/trx/trx.h"
#include "menu/menudisplay.h"
#include "objects/aiagent.h"
#include "objects/block.h"
#include "objects/laraobject.h"
#include "objects/modelobject.h"
#include "objects/objectfactory.h"
#include "objects/pickupobject.h"
#include "objects/tallblock.h"
#include "presenter.h"
#include "render/renderpipeline.h"
#include "render/scene/csm.h"
#include "render/scene/materialmanager.h"
#include "render/scene/renderer.h"
#include "render/scene/rendervisitor.h"
#include "render/scene/scene.h"
#include "render/scene/screenoverlay.h"
#include "render/textureanimator.h"
#include "script/reflection.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"
#include "throttler.h"
#include "tracks_tr1.h"
#include "ui/label.h"
#include "ui/ui.h"
#include "world.h"

#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/range/adaptor/map.hpp>
#include <filesystem>
#include <gl/font.h>
#include <glm/gtx/norm.hpp>
#include <locale>
#include <numeric>
#include <pybind11/embed.h>
#include <pybind11/stl.h>

namespace engine
{
namespace
{
std::shared_ptr<pybind11::scoped_interpreter> createScriptEngine(const std::filesystem::path& rootPath)
{
  auto interpreter = std::make_shared<pybind11::scoped_interpreter>();
  pybind11::module::import("sys").attr("path").cast<pybind11::list>().append(
    std::filesystem::absolute(rootPath).string());
  return interpreter;
}
} // namespace

Engine::Engine(const std::filesystem::path& rootPath, bool fullscreen, const glm::ivec2& resolution)
    : m_rootPath{rootPath}
    , m_presenter{std::make_shared<Presenter>(rootPath, fullscreen, resolution)}
    , m_scriptEngine{createScriptEngine(rootPath)}
{
  if(std::filesystem::is_regular_file(m_rootPath / "config.yaml"))
  {
    serialization::YAMLDocument<true> doc{m_rootPath / "config.yaml"};
    doc.load("config", m_engineConfig, m_engineConfig);
    m_presenter->apply(m_engineConfig.renderSettings);
  }

  try
  {
    pybind11::eval_file((m_rootPath / "scripts" / "main.py").string());
  }
  catch(std::exception& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.py: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load main.py"));
  }

  m_language = std::use_facet<boost::locale::info>(boost::locale::generator()("")).language();
  BOOST_LOG_TRIVIAL(info) << "Detected user's language is " << m_language;
  if(const std::optional overrideLanguage = core::get<std::string>(pybind11::globals(), "language_override"))
  {
    m_language = overrideLanguage.value();
    BOOST_LOG_TRIVIAL(info) << "Language override is " << m_language;
  }

  m_presenter->getInputHandler().setMapping(core::get<hid::InputMapping>(pybind11::globals(), "input_mapping").value());

  for(const auto& [key, name] : EnumUtil<I18n>::all())
  {
    const auto values = pybind11::globals()["i18n"][pybind11::cast(key)];
    Expects(!values.is_none());
    if(const auto loc = core::get<std::string>(values, m_language))
    {
      m_i18n.emplace(key, loc.value());
    }
    else if(const auto en = core::get<std::string>(values, "en"))
    {
      m_i18n.emplace(key, en.value());
    }
    else
    {
      BOOST_LOG_TRIVIAL(error) << "Missing i18n: " << name;
      m_i18n.emplace(key, std::string{"MISSING i18n: "} + name);
    }
  }

  m_glidos = loadGlidosPack();
}

Engine::~Engine()
{
  serialization::YAMLDocument<false> doc{m_rootPath / "config.yaml"};
  doc.save("config", m_engineConfig, m_engineConfig);
  doc.write();
}

std::pair<RunResult, std::optional<size_t>> Engine::run(World& world, bool isCutscene, bool allowSave)
{
  gl::Framebuffer::unbindAll();

  const bool godMode
    = core::get<bool>(core::get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "godMode")
        .value_or(false);

  const bool allAmmoCheat
    = core::get<bool>(core::get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}),
                      "allAmmoCheat")
        .value_or(false);

  std::shared_ptr<menu::MenuDisplay> menu;
  Throttler throttler;
  while(true)
  {
    if(m_presenter->shouldClose())
    {
      return {RunResult::ExitApp, std::nullopt};
    }

    if(world.levelFinished())
    {
      return {RunResult::NextLevel, std::nullopt};
    }

    throttler.wait();
    if(!m_presenter->preFrame())
      continue;

    if(menu != nullptr)
    {
      ui::Ui ui{world.getPresenter().getMaterialManager()->getScreenSpriteTextured(),
                world.getPresenter().getMaterialManager()->getScreenSpriteColorRect(),
                world.getPalette()};
      menu->display(ui, world);
      render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
      m_presenter->getScreenOverlay().render(context);
      ui.render(m_presenter->getViewport());
      m_presenter->swapBuffers();
      switch(menu->result)
      {
      case menu::MenuResult::None: break;
      case menu::MenuResult::Closed: menu.reset(); break;
      case menu::MenuResult::ExitToTitle: return {RunResult::TitleLevel, std::nullopt};
      case menu::MenuResult::ExitGame: return {RunResult::ExitApp, std::nullopt};
      case menu::MenuResult::NewGame: return {RunResult::NextLevel, std::nullopt};
      case menu::MenuResult::LaraHome: return {RunResult::LaraHomeLevel, std::nullopt};
      case menu::MenuResult::RequestLoad:
        Expects(menu->requestLoad.has_value());
        return {RunResult::RequestLoad, menu->requestLoad};
      }
      continue;
    }

    if(!isCutscene)
    {
      if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Menu))
      {
        menu = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::GameMode, world);
        menu->allowSave = allowSave;
        continue;
      }

      if(allowSave && m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Save))
      {
        world.save("quicksave.yaml");
        throttler.reset();
      }
      else if(m_presenter->getInputHandler().hasDebouncedAction(hid::Action::Load))
      {
        return {RunResult::RequestLoad, std::nullopt};
      }

      if(allAmmoCheat)
        m_inventory.fillAllAmmo();

      world.gameLoop(world.getTitle(), godMode);
    }
    else
    {
      if(!world.cinematicLoop())
        return {RunResult::NextLevel, std::nullopt};
    }
  }
}

std::pair<RunResult, std::optional<size_t>> Engine::runTitleMenu(World& world)
{
  gl::Framebuffer::unbindAll();

  m_language = std::use_facet<boost::locale::info>(boost::locale::generator()("")).language();
  BOOST_LOG_TRIVIAL(info) << "Detected user's language is " << m_language;
  if(const std::optional overrideLanguage = core::get<std::string>(pybind11::globals(), "language_override"))
  {
    m_language = overrideLanguage.value();
    BOOST_LOG_TRIVIAL(info) << "Language override is " << m_language;
  }
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

    ui::Ui ui{world.getPresenter().getMaterialManager()->getScreenSpriteTextured(),
              world.getPresenter().getMaterialManager()->getScreenSpriteColorRect(),
              world.getPalette()};
    menu->display(ui, world);
    render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
    m_presenter->getScreenOverlay().render(context);
    ui.render(m_presenter->getViewport());
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
      return {RunResult::RequestLoad, menu->requestLoad};
    }
  }
}

std::pair<RunResult, std::optional<size_t>> Engine::runLevelSequenceItem(script::LevelSequenceItem& item)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  return item.run(*this);
}

std::pair<RunResult, std::optional<size_t>> Engine::runLevelSequenceItemFromSave(script::LevelSequenceItem& item,
                                                                                 const std::optional<size_t>& slot)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  return item.runFromSave(*this, slot);
}

std::unique_ptr<loader::trx::Glidos> Engine::loadGlidosPack() const
{
  if(const auto getGlidosPack = core::get<pybind11::handle>(pybind11::globals(), "getGlidosPack"))
  {
    const auto pack = getGlidosPack.value()();
    if(pack.is_none())
      return nullptr;

    auto glidosPack = pack.cast<std::string>();
    if(!std::filesystem::is_directory(glidosPack))
      return nullptr;

    m_presenter->drawLoadingScreen(i18n(I18n::LoadingGlidos));
    return std::make_unique<loader::trx::Glidos>(m_rootPath / glidosPack,
                                                 [this](const std::string& s) { m_presenter->drawLoadingScreen(s); });
  }

  return nullptr;
}

std::string Engine::i18n(I18n key) const
{
  return m_i18n.at(key);
}

SavegameMeta Engine::getSavegameMeta(const std::filesystem::path& filename) const
{
  serialization::YAMLDocument<true> doc{getSavegamePath() / filename};
  SavegameMeta meta{};
  doc.load("meta", meta, meta);
  return meta;
}
} // namespace engine
