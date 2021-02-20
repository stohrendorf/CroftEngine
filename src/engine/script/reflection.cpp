#include "reflection.h"

#include "engine/cameracontroller.h"
#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/objects/modelobject.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "loader/file/level/level.h"

#include <boost/format.hpp>
#include <boost/range/adaptors.hpp>

namespace engine::script
{
namespace
{
std::filesystem::path getLocalLevelPath(const std::string& basename)
{
  return std::filesystem::path{"data"} / "tr1" / "data" / (basename + ".PHD");
}

std::unique_ptr<loader::file::level::Level>
  loadLevel(Engine& engine, const std::string& basename, const std::string& title)
{
  engine.getPresenter().drawLoadingScreen(engine.i18n(I18n::LoadingLevel, title));
  auto level = loader::file::level::Level::createLoader(engine.getRootPath() / getLocalLevelPath(basename),
                                                        loader::file::level::Game::Unknown);
  level->loadFileData();
  return level;
}
} // namespace

std::pair<RunResult, std::optional<size_t>> Video::run(Engine& engine)
{
  engine.getPresenter().playVideo(engine.getRootPath() / "data/tr1/fmv" / m_name);
  return {RunResult::NextLevel, std::nullopt};
}

std::pair<RunResult, std::optional<size_t>> Cutscene::run(Engine& engine)
{
  auto world = std::make_unique<World>(engine,
                                       loadLevel(engine, m_name, m_name),
                                       std::string{},
                                       m_track,
                                       false,
                                       std::unordered_map<TR1ItemId, size_t>{},
                                       std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>{});

  world->getCameraController().setEyeRotation(0_deg, m_cameraRot);
  auto pos = world->getCameraController().getTRPosition().position;
  if(m_cameraPosX.has_value())
    pos.X = m_cameraPosX.value();
  if(m_cameraPosZ.has_value())
    pos.Z = m_cameraPosZ.value();

  world->getCameraController().setPosition(pos);

  if(m_flipRooms)
    world->swapAllRooms();

  if(m_gunSwap)
  {
    const auto& laraPistol = world->findAnimatedModelForType(TR1ItemId::LaraPistolsAnim);
    Expects(laraPistol != nullptr);
    for(const auto& object : world->getObjectManager().getObjects() | boost::adaptors::map_values)
    {
      if(object->m_state.type != TR1ItemId::CutsceneActor1)
        continue;

      auto m = std::dynamic_pointer_cast<objects::ModelObject>(object.get());
      Expects(m != nullptr);
      m->getSkeleton()->setMeshPart(1, laraPistol->bones[1].mesh);
      m->getSkeleton()->setMeshPart(4, laraPistol->bones[4].mesh);
      m->getSkeleton()->rebuildMesh();
    }
  }

  return engine.run(*world, true, false);
}

std::unique_ptr<engine::World> Level::loadWorld(Engine& engine)
{
  engine.getPresenter().debounceInput();

  auto titleIt = m_titles.find(engine.getLanguage());
  if(titleIt == m_titles.end())
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing level title translation, falling back to language en";
    titleIt = m_titles.find("en");
  }
  if(titleIt == m_titles.end())
    BOOST_LOG_TRIVIAL(error) << "Missing level title";

  const auto title = titleIt == m_titles.end() ? "NO TRANSLATION - " + m_name : titleIt->second;

  auto inventory = m_inventory;
  for(const auto& drop : m_dropInventory)
    inventory.erase(drop);

  return std::make_unique<World>(engine,
                                 loadLevel(engine, m_name, util::unescape(title)),
                                 title,
                                 m_track,
                                 m_useAlternativeLara,
                                 inventory,
                                 m_itemTitles);
}

bool Level::isLevel(const std::filesystem::path& path) const
{
  return getLocalLevelPath(m_name) == path;
}

std::pair<RunResult, std::optional<size_t>> Level::run(Engine& engine)
{
  auto world = loadWorld(engine);
  return engine.run(*world, false, m_allowSave);
}

std::pair<RunResult, std::optional<size_t>> Level::runFromSave(Engine& engine, const std::optional<size_t>& slot)
{
  Expects(m_allowSave);
  auto world = loadWorld(engine);
  if(slot.has_value())
    world->load(slot.value());
  else
    world->load("quicksave.yaml");
  return engine.run(*world, false, m_allowSave);
}

std::pair<RunResult, std::optional<size_t>> TitleMenu::run(Engine& engine)
{
  auto world = loadWorld(engine);
  return engine.runTitleMenu(*world);
}
} // namespace engine::script
