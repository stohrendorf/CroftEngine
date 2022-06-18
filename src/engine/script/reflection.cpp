#include "reflection.h"

#include "core/genericvec.h"
#include "core/i18n.h"
#include "core/id.h"
#include "engine/cameracontroller.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
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
#include "util.h"
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
#include <gl/sampler.h>
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
std::filesystem::path getAssetPath(const Engine& engine, const std::filesystem::path& localPath)
{
  return engine.getAssetDataPath() / localPath;
}

std::unique_ptr<loader::file::level::Level>
  loadLevel(Engine& engine, const std::string& localPath, const std::string& title)
{
  engine.getPresenter().drawLoadingScreen(_("Loading %1%", title));
  auto level
    = loader::file::level::Level::createLoader(getAssetPath(engine, localPath), loader::file::level::Game::Unknown);
  level->loadFileData();
  return level;
}
} // namespace

std::pair<RunResult, std::optional<size_t>> Video::run(Engine& engine,
                                                       const std::shared_ptr<Player>& /*player*/,
                                                       const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  for(const auto& path : m_paths)
  {
    if(auto asset = getAssetPath(engine, path); std::filesystem::is_regular_file(asset))
    {
      engine.getPresenter().playVideo(getAssetPath(engine, asset));
      break;
    }
  }
  return {RunResult::NextLevel, std::nullopt};
}

std::vector<std::filesystem::path> Video::getFilepathsIfInvalid(const Engine& engine) const
{
  for(const auto& path : m_paths)
    if(std::filesystem::is_regular_file(getAssetPath(engine, path)))
      return {};
  return m_paths;
}

std::pair<RunResult, std::optional<size_t>>
  Cutscene::run(Engine& engine, const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& levelStartPlayer)
{
  auto world
    = std::make_unique<world::World>(engine,
                                     loadLevel(engine, m_name, m_name),
                                     std::string{},
                                     m_track,
                                     false,
                                     std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>{},
                                     player,
                                     levelStartPlayer,
                                     false);

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

std::vector<std::filesystem::path> Cutscene::getFilepathsIfInvalid(const Engine& engine) const
{
  if(std::filesystem::is_regular_file(getAssetPath(engine, m_name)))
    return {};
  return {std::filesystem::path{m_name}};
}

std::unique_ptr<world::World> Level::loadWorld(Engine& engine,
                                               const std::shared_ptr<Player>& player,
                                               const std::shared_ptr<Player>& levelStartPlayer,
                                               bool fromSave)
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

  for(const auto& [type, qty] : engine.getScriptEngine().getGameflow().getCheatInventory())
    player->getInventory().put(type.cast<TR1ItemId>(), nullptr, qty.cast<size_t>());

  auto world = std::make_unique<world::World>(engine,
                                              loadLevel(engine, m_name, util::unescape(title)),
                                              title,
                                              m_track,
                                              m_useAlternativeLara,
                                              m_itemTitles,
                                              player,
                                              levelStartPlayer,
                                              fromSave);

  auto replace = [&world, &player](TR1ItemId meshType, TR1ItemId spriteType, TR1ItemId replacement)
  {
    if(player->getInventory().count(meshType) > 0 || player->getInventory().count(spriteType))
    {
      world->getObjectManager().replaceItems(spriteType, replacement, *world);
    }
  };
  replace(TR1ItemId::Shotgun, TR1ItemId::ShotgunSprite, TR1ItemId::ShotgunAmmoSprite);
  replace(TR1ItemId::Uzis, TR1ItemId::UzisSprite, TR1ItemId::UziAmmoSprite);
  replace(TR1ItemId::Magnums, TR1ItemId::MagnumsSprite, TR1ItemId::MagnumAmmoSprite);

  return world;
}

bool Level::isLevel(const std::filesystem::path& path) const
{
  return util::preferredEqual(std::filesystem::path(m_name), path);
}

std::pair<RunResult, std::optional<size_t>>
  Level::run(Engine& engine, const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& levelStartPlayer)
{
  player->requestedWeaponType = m_defaultWeapon;
  player->selectedWeaponType = m_defaultWeapon;

  if(engine.getEngineConfig()->restoreHealth)
    player->laraHealth = core::LaraHealth;

  auto world = loadWorld(engine, player, levelStartPlayer, false);
  return engine.run(*world, false, m_allowSave);
}

std::pair<RunResult, std::optional<size_t>> Level::runFromSave(Engine& engine,
                                                               const std::optional<size_t>& slot,
                                                               const std::shared_ptr<Player>& player,
                                                               const std::shared_ptr<Player>& levelStartPlayer)
{
  Expects(m_allowSave);
  player->getInventory().clear();
  auto world = loadWorld(engine, player, levelStartPlayer, true);
  world->load(slot);
  return engine.run(*world, false, m_allowSave);
}

std::vector<std::filesystem::path> Level::getFilepathsIfInvalid(const Engine& engine) const
{
  if(std::filesystem::is_regular_file(getAssetPath(engine, m_name)))
    return {};
  return {std::filesystem::path{m_name}};
}

std::pair<RunResult, std::optional<size_t>>
  TitleMenu::run(Engine& engine, const std::shared_ptr<Player>& player, const std::shared_ptr<Player>& levelStartPlayer)
{
  player->getInventory().clear();
  auto world = loadWorld(engine, player, levelStartPlayer, false);
  return engine.runTitleMenu(*world);
}

TitleMenu::TitleMenu(const std::string& name,
                     bool useAlternativeLara,
                     const std::unordered_map<std::string, std::string>& titles,
                     const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
                     std::optional<TR1TrackId> track)
    : Level{name, useAlternativeLara, titles, itemTitles, track, false, WeaponType::None}
{
}

SplashScreen::SplashScreen(std::string path, int durationSeconds, int fadeInDurationSeconds, int fadeOutDurationSeconds)
    : m_path{std::move(path)}
    , m_durationSeconds{durationSeconds}
    , m_fadeInDurationSeconds{fadeInDurationSeconds}
    , m_fadeOutDurationSeconds{fadeOutDurationSeconds}
{
  Expects(m_durationSeconds > 0);
  Expects(m_fadeInDurationSeconds >= 0);
  Expects(m_fadeOutDurationSeconds >= 0);
}

SplashScreen::~SplashScreen() = default;

std::pair<RunResult, std::optional<size_t>> SplashScreen::run(Engine& engine,
                                                              const std::shared_ptr<Player>& /*player*/,
                                                              const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  Throttler throttler{};

  glm::ivec2 size{-1, -1};
  auto image = gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(getAssetPath(engine, m_path))}.toTexture(m_path.string()),
    gsl::make_unique<gl::Sampler>(m_path.string() + "-sampler"));
  std::shared_ptr<render::scene::Mesh> mesh;

  auto& presenter = engine.getPresenter();
  float alpha = 0;
  auto updateSize = [this, &size, &presenter, &mesh, &image, &alpha]()
  {
    if(size == presenter.getRenderViewport())
      return;

    size = presenter.getRenderViewport();

    // scale splash image so that its aspect ratio is preserved, but is completely visible
    const auto targetSize = glm::vec2{size};
    const auto sourceSize = glm::vec2{image->getTexture()->size()};
    const float splashScale = std::min(targetSize.x / sourceSize.x, targetSize.y / sourceSize.y);

    auto scaledSourceSize = sourceSize * splashScale;
    auto sourceOffset = (targetSize - scaledSourceSize) / 2.0f;
    mesh = render::scene::createScreenQuad(
      sourceOffset, scaledSourceSize, presenter.getMaterialManager()->getBackdrop(true), m_path.string());
    mesh->bind("u_input",
               [&image](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(image);
               });
    mesh->bind("u_alphaMultiplier",
               [&alpha](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
               {
                 uniform.set(alpha);
               });
  };

  render::scene::RenderContext context{render::scene::RenderMode::Full, std::nullopt};

  auto renderFrame = [&presenter, &updateSize, &mesh, &throttler, &context]() -> bool
  {
    if(!presenter.preFrame() || presenter.shouldClose())
      return false;

    if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Menu))
      return false;

    updateSize();
    gsl_Assert(mesh != nullptr);

    gl::Framebuffer::unbindAll();
    mesh->getRenderState().setViewport(presenter.getDisplayViewport());
    mesh->render(nullptr, context);
    presenter.updateSoundEngine();
    presenter.swapBuffers();

    throttler.wait();
    return true;
  };

  for(const auto end = std::chrono::high_resolution_clock::now() + std::chrono::seconds{m_fadeInDurationSeconds};
      std::chrono::high_resolution_clock::now() < end;)
  {
    const auto time = gsl::narrow_cast<float>(
      std::chrono::duration_cast<std::chrono::milliseconds>(end - std::chrono::high_resolution_clock::now()).count());
    const auto duration = gsl::narrow_cast<float>(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds{m_fadeInDurationSeconds}).count());
    alpha = std::clamp(1 - time / duration, 0.0f, 1.0f);
    BOOST_LOG_TRIVIAL(debug) << "time=" << time << ", duration=" << duration << ", alpha=" << alpha;
    if(!renderFrame())
      return {RunResult::NextLevel, std::nullopt};
  }

  alpha = 1;

  for(const auto end = std::chrono::high_resolution_clock::now() + std::chrono::seconds{m_durationSeconds};
      std::chrono::high_resolution_clock::now() < end;)
  {
    if(!renderFrame())
      return {RunResult::NextLevel, std::nullopt};
  }

  for(const auto end = std::chrono::high_resolution_clock::now() + std::chrono::seconds{m_fadeOutDurationSeconds};
      std::chrono::high_resolution_clock::now() < end;)
  {
    const auto time = gsl::narrow_cast<float>(
      std::chrono::duration_cast<std::chrono::milliseconds>(end - std::chrono::high_resolution_clock::now()).count());
    const auto duration = gsl::narrow_cast<float>(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds{m_fadeOutDurationSeconds}).count());
    alpha = std::clamp(time / duration, 0.0f, 1.0f);
    if(!renderFrame())
      return {RunResult::NextLevel, std::nullopt};
  }

  return {RunResult::NextLevel, std::nullopt};
}

std::vector<std::filesystem::path> SplashScreen::getFilepathsIfInvalid(const Engine& engine) const
{
  if(std::filesystem::is_regular_file(getAssetPath(engine, m_path)))
    return {};
  return {std::filesystem::path{m_path}};
}

std::pair<RunResult, std::optional<size_t>> LevelSequenceItem::runFromSave(Engine&,
                                                                           const std::optional<size_t>&,
                                                                           const std::shared_ptr<Player>&,
                                                                           const std::shared_ptr<Player>&)
{
  BOOST_LOG_TRIVIAL(error) << "Cannot run from save";
  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot run from save"));
}

std::pair<RunResult, std::optional<size_t>> ModifyInventory::run(Engine& /*engine*/,
                                                                 const std::shared_ptr<Player>& player,
                                                                 const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  for(const auto& [type, qty] : m_addInventory)
    player->getInventory().put(type, nullptr, qty);
  for(const auto& type : m_dropInventory)
    player->getInventory().drop(type);
  return {RunResult::NextLevel, std::nullopt};
}

bool Gameflow::isGodMode() const
{
  return get<bool>(m_cheats, "godMode").value_or(false);
}

bool Gameflow::hasAllAmmoCheat() const
{
  return get<bool>(m_cheats, "allAmmoCheat").value_or(false);
}

pybind11::dict Gameflow::getCheatInventory() const
{
  return get<pybind11::dict>(m_cheats, "inventory").value_or(pybind11::dict{});
}

std::vector<std::filesystem::path> Gameflow::getInvalidFilepaths(const Engine& engine) const
{
  std::vector<std::filesystem::path> result;
  for(const auto& track : m_tracks)
    for(const auto invalid : track.second->getFilepathsIfInvalid(engine))
      result.emplace_back(invalid);
  for(const auto& levelSequenceItem : m_levelSequence)
  {
    Expects(levelSequenceItem != nullptr);
    for(const auto& invalid : levelSequenceItem->getFilepathsIfInvalid(engine))
      result.emplace_back(invalid);
  }
  Expects(m_titleMenu != nullptr);
  for(const auto& invalid : m_titleMenu->getFilepathsIfInvalid(engine))
    result.emplace_back(invalid);
  for(const auto& levelSequenceItem : m_laraHome)
  {
    Expects(levelSequenceItem != nullptr);
    for(const auto& invalid : levelSequenceItem->getFilepathsIfInvalid(engine))
      result.emplace_back(invalid);
  }
  for(const auto& levelSequenceItem : m_earlyBoot)
  {
    Expects(levelSequenceItem != nullptr);
    for(const auto& invalid : levelSequenceItem->getFilepathsIfInvalid(engine))
      result.emplace_back(invalid);
  }
  if(!std::filesystem::is_regular_file(getAssetPath(engine, m_titleMenuBackdrop)))
    result.emplace_back(m_titleMenuBackdrop);
  return result;
}

std::vector<std::filesystem::path> TrackInfo::getFilepathsIfInvalid(const Engine& engine) const
{
  for(const auto& path : paths)
  {
    if(std::filesystem::is_regular_file(getAssetPath(engine, path)))
      return {};
  }

  return paths;
}

std::filesystem::path TrackInfo::getFirstValidAlternative(const std::filesystem::path& rootPath) const
{
  for(const auto& path : paths)
  {
    if(std::filesystem::is_regular_file(rootPath / path))
      return path;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("no valid alternative found"));
}
} // namespace engine::script
