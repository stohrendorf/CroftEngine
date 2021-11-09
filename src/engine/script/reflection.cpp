#include "reflection.h"

#include "core/i18n.h"
#include "core/vec.h"
#include "engine/cameracontroller.h"
#include "engine/engine.h"
#include "engine/inventory.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/objects/modelobject.h"
#include "engine/objects/object.h"
#include "engine/objects/objectstate.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/throttler.h"
#include "engine/weapontype.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "loader/file/level/game.h"
#include "loader/file/level/level.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/renderer.h"
#include "render/scene/rendermode.h"
#include "scriptengine.h"
#include "util/fsutil.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <gl/cimgwrapper.h>
#include <gl/framebuffer.h>
#include <gl/pixel.h>
#include <gl/program.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <iosfwd>
#include <map>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h> // IWYU pragma: keep
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <vector>

namespace render::scene
{
class Node;
}

namespace engine::script
{
namespace
{
std::filesystem::path getLocalLevelPath(const std::string& basename)
{
  return std::filesystem::path{"data"} / "tr1" / "DATA" / (basename + ".PHD");
}

std::unique_ptr<loader::file::level::Level>
  loadLevel(Engine& engine, const std::string& basename, const std::string& title)
{
  engine.getPresenter().drawLoadingScreen(_("Loading %1%", title));
  auto level = loader::file::level::Level::createLoader(engine.getUserDataPath() / getLocalLevelPath(basename),
                                                        loader::file::level::Game::Unknown);
  level->loadFileData();
  return level;
}
} // namespace

std::pair<RunResult, std::optional<size_t>> Video::run(Engine& engine, const std::shared_ptr<Player>& /*player*/)
{
  engine.getPresenter().playVideo(engine.getUserDataPath() / "data" / "tr1" / "FMV" / m_name);
  return {RunResult::NextLevel, std::nullopt};
}

std::pair<RunResult, std::optional<size_t>> Cutscene::run(Engine& engine, const std::shared_ptr<Player>& player)
{
  auto world
    = std::make_unique<world::World>(engine,
                                     loadLevel(engine, m_name, m_name),
                                     std::string{},
                                     0,
                                     m_track,
                                     false,
                                     std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>{},
                                     player);

  world->getCameraController().setEyeRotation(0_deg, m_cameraRot);
  auto pos = world->getCameraController().getTRLocation().position;
  if(m_cameraPosX.has_value())
    pos.X = *m_cameraPosX;
  if(m_cameraPosZ.has_value())
    pos.Z = *m_cameraPosZ;

  world->getCameraController().setPosition(pos);

  if(m_flipRooms)
    world->swapAllRooms();

  if(m_weaponSwap)
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

std::unique_ptr<world::World> Level::loadWorld(Engine& engine, const std::shared_ptr<Player>& player)
{
  engine.getPresenter().debounceInput();

  auto titleIt = m_titles.find(engine.getLocaleWithoutEncoding());
  if(titleIt == m_titles.end())
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing level title translation for language " << engine.getLocaleWithoutEncoding()
                               << ", falling back to language en_GB";
    titleIt = m_titles.find("en_GB");
  }
  if(titleIt == m_titles.end())
    BOOST_LOG_TRIVIAL(error) << "Missing level title";

  const auto title = titleIt == m_titles.end() ? "NO TRANSLATION - " + m_name : titleIt->second;

  player->resetStats();
  for(const auto& [type, qty] : m_inventory)
    player->getInventory().put(type, qty);
  for(const auto& type : m_dropInventory)
    player->getInventory().drop(type);

  for(const auto& [type, qty] : engine.getScriptEngine().getCheatInventory())
    player->getInventory().put(type.cast<TR1ItemId>(), qty.cast<size_t>());

  return std::make_unique<world::World>(engine,
                                        loadLevel(engine, m_name, util::unescape(title)),
                                        title,
                                        m_secrets,
                                        m_track,
                                        m_useAlternativeLara,
                                        m_itemTitles,
                                        player);
}

bool Level::isLevel(const std::filesystem::path& path) const
{
  return util::preferredEqual(getLocalLevelPath(m_name), path);
}

std::pair<RunResult, std::optional<size_t>> Level::run(Engine& engine, const std::shared_ptr<Player>& player)
{
  player->requestedWeaponType = m_defaultWeapon;
  player->selectedWeaponType = m_defaultWeapon;
  auto world = loadWorld(engine, player);
  return engine.run(*world, false, m_allowSave);
}

std::pair<RunResult, std::optional<size_t>>
  Level::runFromSave(Engine& engine, const std::optional<size_t>& slot, const std::shared_ptr<Player>& player)
{
  Expects(m_allowSave);
  player->getInventory().clear();
  auto world = loadWorld(engine, player);
  world->load(slot);
  return engine.run(*world, false, m_allowSave);
}

std::pair<RunResult, std::optional<size_t>> TitleMenu::run(Engine& engine, const std::shared_ptr<Player>& player)
{
  player->getInventory().clear();
  auto world = loadWorld(engine, player);
  return engine.runTitleMenu(*world);
}

TitleMenu::TitleMenu(const std::string& name,
                     bool useAlternativeLara,
                     const std::unordered_map<std::string, std::string>& titles,
                     const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
                     const std::unordered_map<TR1ItemId, size_t>& inventory,
                     const std::unordered_set<TR1ItemId>& dropInventory,
                     std::optional<TR1TrackId> track)
    : Level{name, 0, useAlternativeLara, titles, itemTitles, inventory, dropInventory, track, false, WeaponType::None}
{
}

SplashScreen::SplashScreen(std::string path, int durationSeconds)
    : m_path{std::move(path)}
    , m_durationSeconds{durationSeconds}
{
  Expects(m_durationSeconds > 0);
}

SplashScreen::~SplashScreen() = default;

std::pair<RunResult, std::optional<size_t>> SplashScreen::run(Engine& engine, const std::shared_ptr<Player>& /*player*/)
{
  const auto end = std::chrono::high_resolution_clock::now() + std::chrono::seconds{m_durationSeconds};
  Throttler throttler{};

  glm::ivec2 size{-1, -1};
  auto image = gslu::make_nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(engine.getUserDataPath() / "data" / "tr1" / "DATA" / m_path)}.toTexture(
      m_path.string()),
    gslu::make_nn_unique<gl::Sampler>(m_path.string() + "-sampler"));
  std::shared_ptr<render::scene::Mesh> mesh;

  render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};
  while(std::chrono::high_resolution_clock::now() < end)
  {
    auto& presenter = engine.getPresenter();
    if(presenter.update() || presenter.shouldClose())
      break;

    presenter.getInputHandler().update();
    if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Menu))
      break;

    if(size != presenter.getRenderViewport())
    {
      size = presenter.getRenderViewport();

      // scale splash image so that its aspect ratio is preserved, but is completely visible
      const auto targetSize = glm::vec2{size};
      const auto sourceSize = glm::vec2{image->getTexture()->size()};
      const float splashScale = std::min(targetSize.x / sourceSize.x, targetSize.y / sourceSize.y);

      auto scaledSourceSize = sourceSize * splashScale;
      auto sourceOffset = (targetSize - scaledSourceSize) / 2.0f;
      mesh = render::scene::createScreenQuad(
        sourceOffset, scaledSourceSize, presenter.getMaterialManager()->getBackdrop(), m_path.string());
      mesh->bind(
        "u_input",
        [&image](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
        {
          uniform.set(image);
        });
    }

    Ensures(mesh != nullptr);

    gl::Framebuffer::unbindAll();
    presenter.getRenderer().clear(
      gl::api::ClearBufferMask::ColorBufferBit | gl::api::ClearBufferMask::DepthBufferBit, {0, 0, 0, 0}, 1);
    mesh->render(nullptr, context);
    presenter.updateSoundEngine();
    presenter.swapBuffers();

    throttler.wait();
  }

  return {RunResult::NextLevel, std::nullopt};
}

std::pair<RunResult, std::optional<size_t>>
  LevelSequenceItem::runFromSave(Engine&, const std::optional<size_t>&, const std::shared_ptr<Player>&)
{
  BOOST_LOG_TRIVIAL(error) << "Cannot run from save";
  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot run from save"));
}
} // namespace engine::script
