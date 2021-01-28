#include "engine.h"

#include "audio/tracktype.h"
#include "core/pybindmodule.h"
#include "engine/ai/ai.h"
#include "engine/audioengine.h"
#include "floordata/floordata.h"
#include "hid/inputhandler.h"
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
#include "throttler.h"
#include "tracks_tr1.h"
#include "ui/label.h"
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
  try
  {
    pybind11::eval_file((m_rootPath / "scripts" / "main.py").string());
  }
  catch(std::exception& e)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load main.py: " << e.what();
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load main.py"));
  }

  m_glidos = loadGlidosPack();
}

RunResult Engine::run(World& world, bool isCutscene)
{
  gl::Framebuffer::unbindAll();

  m_language = std::use_facet<boost::locale::info>(boost::locale::generator()("")).language();
  BOOST_LOG_TRIVIAL(info) << "Detected user's language is " << m_language;
  if(const std::optional overrideLanguage = core::get<std::string>(pybind11::globals(), "language_override"))
  {
    m_language = overrideLanguage.value();
    BOOST_LOG_TRIVIAL(info) << "Language override is " << m_language;
  }

  const bool godMode
    = core::get<bool>(core::get<pybind11::dict>(pybind11::globals(), "cheats").value_or(pybind11::dict{}), "godMode")
        .value_or(false);

  std::shared_ptr<menu::MenuDisplay> menu;
  Throttler throttler;
  while(true)
  {
    if(m_presenter->shouldClose())
    {
      return RunResult::ExitApp;
    }

    if(world.levelFinished())
    {
      return RunResult::NextLevel;
    }

    throttler.wait();

    if(menu != nullptr)
    {
      m_presenter->preFrame();
      menu->display(*m_presenter->getScreenOverlay().getImage(), world);
      render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
      m_presenter->getScreenOverlay().render(context);
      m_presenter->swapBuffers();
      switch(menu->result)
      {
      case menu::MenuResult::None: break;
      case menu::MenuResult::Closed: menu.reset(); break;
      case menu::MenuResult::ExitToTitle: return RunResult::TitleLevel;
      case menu::MenuResult::ExitGame: return RunResult::ExitApp;
      case menu::MenuResult::NewGame: return RunResult::NextLevel;
      case menu::MenuResult::LaraHome: return RunResult::LaraHomeLevel;
      }
      continue;
    }

    if(!isCutscene)
    {
      if(m_presenter->getInputHandler().getInputState().menu.justChangedTo(true))
      {
        menu = std::make_shared<menu::MenuDisplay>(menu::InventoryMode::GameMode, world);
        continue;
      }

      if(m_presenter->getInputHandler().getInputState().save.justChangedTo(true))
      {
        world.save("quicksave.yaml");
        throttler.reset();
      }
      else if(m_presenter->getInputHandler().getInputState().load.justChangedTo(true))
      {
        world.load("quicksave.yaml");
        throttler.reset();
      }

      world.gameLoop(world.getTitle(), godMode);
    }
    else
    {
      if(!world.cinematicLoop())
        return RunResult::NextLevel;
    }
  }
}

RunResult Engine::runTitleMenu(World& world)
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
      return RunResult::ExitApp;
    }

    throttler.wait();

    m_presenter->preFrame();
    menu->display(*m_presenter->getScreenOverlay().getImage(), world);
    render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
    m_presenter->getScreenOverlay().render(context);
    m_presenter->swapBuffers();
    switch(menu->result)
    {
    case menu::MenuResult::None: break;
    case menu::MenuResult::Closed: return RunResult::ExitApp;
    case menu::MenuResult::ExitToTitle: return RunResult::TitleLevel;
    case menu::MenuResult::ExitGame: return RunResult::ExitApp;
    case menu::MenuResult::NewGame: return RunResult::NextLevel;
    case menu::MenuResult::LaraHome: return RunResult::LaraHomeLevel;
    }
  }
}

RunResult Engine::runLevelSequenceItem(script::LevelSequenceItem& item)
{
  m_presenter->getSoundEngine()->reset();
  m_presenter->clear();
  return item.run(*this);
}

Engine::~Engine() = default;

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

    m_presenter->drawLoadingScreen("Loading Glidos texture pack");
    return std::make_unique<loader::trx::Glidos>(m_rootPath / glidosPack,
                                                 [this](const std::string& s) { m_presenter->drawLoadingScreen(s); });
  }

  return nullptr;
}
} // namespace engine
