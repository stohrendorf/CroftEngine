#include "reflection.h"

#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world.h"

#include <boost/range/adaptors.hpp>

namespace engine::script
{
namespace
{
std::unique_ptr<loader::file::level::Level> loadLevel(Engine& engine, const std::string& name)
{
  engine.getPresenter().drawLoadingScreen("Loading " + name);
  auto level = loader::file::level::Level::createLoader(
    engine.getRootPath() / "data" / "tr1" / "data" / (name + ".PHD"), loader::file::level::Game::Unknown);
  level->loadFileData();
  return level;
}
} // namespace

RunResult Video::run(Engine& engine)
{
  engine.getPresenter().playVideo(engine.getRootPath() / "data/tr1/fmv" / m_name);
  return RunResult::NextLevel;
}

RunResult Cutscene::run(Engine& engine)
{
  auto world = std::make_unique<World>(
    engine, loadLevel(engine, m_name), std::string{}, m_track, false, std::unordered_map<TR1ItemId, size_t>{});

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

  return engine.run(*world, true);
}

RunResult Level::run(Engine& engine)
{
  auto titleIt = m_titles.find(engine.getLanguage());
  if(titleIt == m_titles.end())
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing level title translation, falling back to language en";
    titleIt = m_titles.find("en");
  }
  if(titleIt == m_titles.end())
    BOOST_LOG_TRIVIAL(error) << "Missing level title";

  const auto title = titleIt == m_titles.end() ? "NO TRANSLATION - " + m_name : titleIt->second;
  auto world
    = std::make_unique<World>(engine, loadLevel(engine, m_name), title, m_track, m_useAlternativeLara, m_inventory);

  // FIXME remove the "TITLE" hack
  return m_name == "TITLE" ? engine.runTitleMenu(*world) : engine.run(*world, false);
}
} // namespace engine::script
