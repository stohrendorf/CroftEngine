#include "reflection.h"

#include "audio/soundengine.h"
#include "audio/streamvoice.h"
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
#include "render/material/material.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/scene/mesh.h"
#include "render/scene/rendercontext.h"
#include "render/scene/renderer.h"
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
#include <gl/constants.h>
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
std::unique_ptr<loader::file::level::Level> loadLevel(const gsl::not_null<Engine*>& engine,
                                                      const std::string& localPath,
                                                      const std::string& title,
                                                      loader::file::level::Game game)
{
  engine->getPresenter().drawLoadingScreen(_("Loading %1%", title));
  auto level = loader::file::level::Level::createLoader(engine->getAssetDataPath() / localPath, game);
  level->loadFileData();
  return level;
}
} // namespace

std::pair<RunResult, std::optional<size_t>> Video::run(const gsl::not_null<Engine*>& engine,
                                                       const std::shared_ptr<Player>& /*player*/,
                                                       const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  engine->getPresenter().getSoundEngine()->reset();
  for(const auto& path : m_paths)
  {
    if(auto asset = engine->getAssetDataPath() / path; std::filesystem::is_regular_file(asset))
    {
      engine->getPresenter().playVideo(engine->getAssetDataPath() / asset);
      break;
    }
  }
  return {RunResult::NextLevel, std::nullopt};
}

std::vector<std::filesystem::path> Video::getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const
{
  if(m_optional)
  {
    return {};
  }

  for(const auto& path : m_paths)
    if(std::filesystem::is_regular_file(dataRoot / path))
      return {};
  return m_paths;
}

std::pair<RunResult, std::optional<size_t>> Cutscene::run(const gsl::not_null<Engine*>& engine,
                                                          const std::shared_ptr<Player>& player,
                                                          const std::shared_ptr<Player>& levelStartPlayer)
{
  engine->getPresenter().getSoundEngine()->reset();
  auto world
    = std::make_unique<world::World>(engine,
                                     loadLevel(engine, m_name, m_name, m_game),
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
    gsl_Assert(laraPistol != nullptr);
    for(const auto& object : world->getObjectManager().getObjects() | boost::adaptors::map_values)
    {
      if(object->m_state.type != TR1ItemId::CutsceneActor1)
        continue;

      auto m = std::dynamic_pointer_cast<objects::ModelObject>(object.get());
      gsl_Assert(m != nullptr);
      m->getSkeleton()->setMesh(1, laraPistol->bones[1].mesh);
      m->getSkeleton()->setMesh(4, laraPistol->bones[4].mesh);
      m->getSkeleton()->rebuildMesh();
    }
  }

  return engine->run(*world, true, false);
}

std::vector<std::filesystem::path> Cutscene::getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const
{
  if(std::filesystem::is_regular_file(dataRoot / m_name))
    return {};
  return {std::filesystem::path{m_name}};
}

std::unique_ptr<world::World> Level::loadWorld(const gsl::not_null<Engine*>& engine,
                                               const std::shared_ptr<Player>& player,
                                               const std::shared_ptr<Player>& levelStartPlayer,
                                               bool fromSave)
{
  engine->getPresenter().debounceInput();

  auto titleIt = m_titles.find(engine->getLocaleWithoutEncoding());
  if(titleIt == m_titles.end())
  {
    BOOST_LOG_TRIVIAL(warning) << "Missing level title translation for language " << engine->getLocaleWithoutEncoding()
                               << ", falling back to language en_GB";
    titleIt = m_titles.find("en_GB");
  }
  if(titleIt == m_titles.end())
    BOOST_LOG_TRIVIAL(error) << "Missing level title";

  const auto title = titleIt == m_titles.end() ? "NO TRANSLATION - " + m_name : titleIt->second;

  for(const auto& [type, qty] : engine->getScriptEngine().getGameflow().getCheatInventory())
    player->getInventory().put(type.cast<TR1ItemId>(), nullptr, qty.cast<size_t>());

  auto world = std::make_unique<world::World>(engine,
                                              loadLevel(engine, m_name, util::unescape(title), m_game),
                                              title,
                                              m_ambient,
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

  for(auto a : {false, true})
  {
    for(auto b : {false, true})
    {
      engine->getPresenter()
        .getMaterialManager()
        ->getWorldComposition(a, b)
        ->getUniform("u_waterColor")
        ->set(m_waterColor);
      engine->getPresenter()
        .getMaterialManager()
        ->getWorldComposition(a, b)
        ->getUniform("u_waterDensity")
        ->set(m_waterDensity);
    }
  }

  return world;
}

bool Level::isLevel(const std::filesystem::path& path) const
{
  return util::preferredEqual(std::filesystem::path(m_name), path);
}

std::pair<RunResult, std::optional<size_t>> Level::run(const gsl::not_null<Engine*>& engine,
                                                       const std::shared_ptr<Player>& player,
                                                       const std::shared_ptr<Player>& levelStartPlayer)
{
  if(m_alternativeSplashscreen.has_value()
     && std::filesystem::is_regular_file(engine->getAssetDataPath() / *m_alternativeSplashscreen))
    engine->getPresenter().setSplashImageTextureOverride(engine->getAssetDataPath() / *m_alternativeSplashscreen);
  else
    engine->getPresenter().clearSplashImageTextureOverride();

  engine->getPresenter().getSoundEngine()->reset();
  player->requestedWeaponType = m_defaultWeapon;
  player->selectedWeaponType = m_defaultWeapon;

  if(engine->getEngineConfig()->restoreHealth)
    player->laraHealth = core::LaraHealth;

  auto world = loadWorld(engine, player, levelStartPlayer, false);
  auto result = engine->run(*world, false, m_allowSave);
  engine->getPresenter().clearSplashImageTextureOverride();
  return result;
}

std::pair<RunResult, std::optional<size_t>> Level::runFromSave(const gsl::not_null<Engine*>& engine,
                                                               const std::optional<size_t>& slot,
                                                               const std::shared_ptr<Player>& player,
                                                               const std::shared_ptr<Player>& levelStartPlayer)
{
  gsl_Expects(m_allowSave);

  if(m_alternativeSplashscreen.has_value()
     && std::filesystem::is_regular_file(engine->getAssetDataPath() / *m_alternativeSplashscreen))
    engine->getPresenter().setSplashImageTextureOverride(engine->getAssetDataPath() / *m_alternativeSplashscreen);
  else
    engine->getPresenter().clearSplashImageTextureOverride();

  engine->getPresenter().getSoundEngine()->reset();
  player->getInventory().clear();
  auto world = loadWorld(engine, player, levelStartPlayer, true);
  world->load(slot);
  auto result = engine->run(*world, false, m_allowSave);
  engine->getPresenter().clearSplashImageTextureOverride();
  return result;
}

std::vector<std::filesystem::path> Level::getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const
{
  if(std::filesystem::is_regular_file(dataRoot / m_name))
    return {};
  return {std::filesystem::path{m_name}};
}

std::pair<RunResult, std::optional<size_t>> TitleMenu::run(const gsl::not_null<Engine*>& engine,
                                                           const std::shared_ptr<Player>& player,
                                                           const std::shared_ptr<Player>& levelStartPlayer)
{
  engine->getPresenter().getSoundEngine()->reset();
  player->getInventory().clear();
  auto world = loadWorld(engine, player, levelStartPlayer, false);
  return engine->runTitleMenu(*world);
}

TitleMenu::TitleMenu(const std::string& name,
                     bool useAlternativeLara,
                     const std::unordered_map<std::string, std::string>& titles,
                     const std::unordered_map<std::string, std::unordered_map<TR1ItemId, std::string>>& itemTitles,
                     std::optional<TR1TrackId> ambient,
                     loader::file::level::Game game)
    : Level{name,
            useAlternativeLara,
            titles,
            itemTitles,
            ambient,
            false,
            WeaponType::None,
            DefaultWaterColor,
            DefaultWaterDensity,
            std::nullopt,
            game}
{
}

SplashScreen::SplashScreen(std::string path, int durationSeconds, int fadeInDurationSeconds, int fadeOutDurationSeconds)
    : m_path{std::move(path)}
    , m_durationSeconds{durationSeconds}
    , m_fadeInDurationSeconds{fadeInDurationSeconds}
    , m_fadeOutDurationSeconds{fadeOutDurationSeconds}
{
  gsl_Expects(m_durationSeconds > 0);
  gsl_Expects(m_fadeInDurationSeconds >= 0);
  gsl_Expects(m_fadeOutDurationSeconds >= 0);
}

SplashScreen::~SplashScreen() = default;

std::pair<RunResult, std::optional<size_t>> SplashScreen::run(const gsl::not_null<Engine*>& engine,
                                                              const std::shared_ptr<Player>& /*player*/,
                                                              const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  Throttler throttler{};

  glm::ivec2 size{-1, -1};
  auto image = gsl::make_shared<gl::TextureHandle<gl::Texture2D<gl::PremultipliedSRGBA8>>>(
    gl::CImgWrapper{util::ensureFileExists(engine->getAssetDataPath() / m_path)}.toTexture(m_path.string()),
    gsl::make_unique<gl::Sampler>(m_path.string() + gl::SamplerSuffix));
  std::shared_ptr<render::scene::Mesh> mesh;

  auto& presenter = engine->getPresenter();
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

    const auto scaledSourceSize = sourceSize * splashScale;
    const auto sourceOffset = (targetSize - scaledSourceSize) / 2.0f;
    mesh = render::scene::createScreenQuad(sourceOffset,
                                           scaledSourceSize,
                                           presenter.getMaterialManager()->getBackdrop(true),
                                           render::scene::Translucency::Opaque,
                                           m_path.string());
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

  auto renderFrame = [&presenter, &updateSize, &mesh, &throttler]() -> bool
  {
    if(!presenter.preFrame() || presenter.shouldClose())
      return false;

    if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Menu))
      return false;

    updateSize();
    gsl_Assert(mesh != nullptr);

    mesh->getRenderState().setViewport(presenter.getDisplayViewport());
    presenter.withBackbuffer(
      [&mesh]()
      {
        render::scene::RenderContext context{
          render::material::RenderMode::FullOpaque, std::nullopt, render::scene::Translucency::Opaque};
        mesh->render(nullptr, context);
      });
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

std::vector<std::filesystem::path> SplashScreen::getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const
{
  if(std::filesystem::is_regular_file(dataRoot / m_path))
    return {};
  return {std::filesystem::path{m_path}};
}

std::pair<RunResult, std::optional<size_t>> LevelSequenceItem::runFromSave(const gsl::not_null<Engine*>&,
                                                                           const std::optional<size_t>&,
                                                                           const std::shared_ptr<Player>&,
                                                                           const std::shared_ptr<Player>&)
{
  BOOST_LOG_TRIVIAL(error) << "Cannot run from save";
  BOOST_THROW_EXCEPTION(std::runtime_error("Cannot run from save"));
}

std::pair<RunResult, std::optional<size_t>> ModifyInventory::run(const gsl::not_null<Engine*>& /*engine*/,
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

std::vector<std::filesystem::path> Gameflow::getInvalidFilepaths(const std::filesystem::path& dataRoot) const
{
  std::vector<std::filesystem::path> result;
  for(const auto& track : m_tracks)
    for(auto&& invalid : track.second->getFilepathsIfInvalid(dataRoot))
      result.emplace_back(std::move(invalid));
  for(const auto& levelSequenceItem : m_levelSequence)
  {
    gsl_Assert(levelSequenceItem != nullptr);
    for(auto&& invalid : levelSequenceItem->getFilepathsIfInvalid(dataRoot))
      result.emplace_back(std::move(invalid));
  }
  gsl_Expects(m_titleMenu != nullptr);
  for(auto&& invalid : m_titleMenu->getFilepathsIfInvalid(dataRoot))
    result.emplace_back(std::move(invalid));
  for(const auto& levelSequenceItem : m_laraHome)
  {
    gsl_Assert(levelSequenceItem != nullptr);
    for(auto&& invalid : levelSequenceItem->getFilepathsIfInvalid(dataRoot))
      result.emplace_back(std::move(invalid));
  }
  for(const auto& levelSequenceItem : m_earlyBoot)
  {
    gsl_Assert(levelSequenceItem != nullptr);
    for(auto&& invalid : levelSequenceItem->getFilepathsIfInvalid(dataRoot))
      result.emplace_back(std::move(invalid));
  }
  if(!std::filesystem::is_regular_file(dataRoot / m_titleMenuBackdrop))
    result.emplace_back(m_titleMenuBackdrop);
  return result;
}

std::vector<std::filesystem::path> TrackInfo::getFilepathsIfInvalid(const std::filesystem::path& dataRoot) const
{
  for(const auto& path : paths)
  {
    if(std::filesystem::is_regular_file(dataRoot / path))
      return {};
  }

  return paths;
}

std::filesystem::path TrackInfo::getFirstValidAlternative(const std::filesystem::path& dataRoot) const
{
  for(const auto& path : paths)
  {
    if(std::filesystem::is_regular_file(dataRoot / path))
      return path;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("no valid alternative found"));
}

std::pair<RunResult, std::optional<size_t>> PlayAudioSlot::run(const gsl::not_null<Engine*>& engine,
                                                               const std::shared_ptr<Player>& /*player*/,
                                                               const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  const gsl::not_null trackInfo{engine->getScriptEngine().getGameflow().getTracks().at(m_track)};
  engine->getPresenter().getSoundEngine()->freeSlot(m_slot);
  const auto stream = engine->getPresenter().getSoundEngine()->createStream(
    engine->getAssetDataPath() / trackInfo->getFirstValidAlternative(engine->getAssetDataPath()),
    std::chrono::milliseconds{0});
  stream->setLooping(trackInfo->looping);
  engine->getPresenter().getSoundEngine()->setSlotStream(
    m_slot, stream, engine->getAssetDataPath() / trackInfo->getFirstValidAlternative(engine->getAssetDataPath()));
  stream->play();
  return {RunResult::NextLevel, std::nullopt};
}

std::pair<RunResult, std::optional<size_t>> StopAudioSlot::run(const gsl::not_null<Engine*>& engine,
                                                               const std::shared_ptr<Player>& /*player*/,
                                                               const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  engine->getPresenter().getSoundEngine()->freeSlot(m_slot);
  return {RunResult::NextLevel, std::nullopt};
}

std::pair<RunResult, std::optional<size_t>> ResetSoundEngine::run(const gsl::not_null<Engine*>& engine,
                                                                  const std::shared_ptr<Player>& /*player*/,
                                                                  const std::shared_ptr<Player>& /*levelStartPlayer*/)
{
  engine->getPresenter().getSoundEngine()->reset();
  return {RunResult::NextLevel, std::nullopt};
}
} // namespace engine::script
