#include "menudisplay.h"

#include "core/i18n.h"
#include "core/units.h"
#include "engine/audioengine.h"
#include "engine/cameracontroller.h"
#include "engine/engine.h"
#include "engine/inventory.h"
#include "engine/items_tr1.h"
#include "engine/lighting.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "engine/soundeffects_tr1.h"
#include "engine/weapontype.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "inflateringmenustate.h"
#include "menuobject.h"
#include "menuring.h"
#include "menuringtransform.h"
#include "menustate.h"
#include "qs/qs.h"
#include "render/material/materialmanager.h"
#include "render/pass/framebuffer.h"
#include "render/scene/camera.h"
#include "render/scene/translucency.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "ui/util.h"
#include "util.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <gl/buffer.h>
#include <gl/debuggroup.h>
#include <gl/pixel.h>
#include <gl/renderstate.h>
#include <gl/texture2d.h>
#include <gl/texturedepth.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace menu
{
void MenuDisplay::constructMenuObjectDescription(ui::Ui& ui, engine::world::World& world, const MenuObject& object)
{
  size_t totalItemCount = world.getPlayer().getInventory().count(object.type);
  std::string suffix;

  const auto drawAmmo = [&ui, &world](const engine::Ammo& ammo)
  {
    const ui::Text text{ui::makeAmmoString(ammo.getDisplayString())};
    text.draw(ui,
              world.getEngine().getPresenter().getTrFont(),
              {(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y - RingInfoYMargin - 2 * ui::FontHeight});
  };

  switch(object.type)
  {
  case engine::TR1ItemId::Puzzle1:
  case engine::TR1ItemId::Puzzle1Sprite:
  case engine::TR1ItemId::Puzzle2:
  case engine::TR1ItemId::Puzzle2Sprite:
  case engine::TR1ItemId::Puzzle3:
  case engine::TR1ItemId::Puzzle3Sprite:
  case engine::TR1ItemId::Puzzle4:
  case engine::TR1ItemId::Puzzle4Sprite:
  case engine::TR1ItemId::Key1:
  case engine::TR1ItemId::Key1Sprite:
  case engine::TR1ItemId::Key2:
  case engine::TR1ItemId::Key2Sprite:
  case engine::TR1ItemId::Key3:
  case engine::TR1ItemId::Key3Sprite:
  case engine::TR1ItemId::Key4:
  case engine::TR1ItemId::Key4Sprite:
  case engine::TR1ItemId::Item148:
  case engine::TR1ItemId::Item149:
  case engine::TR1ItemId::ScionPieceCounter:
  case engine::TR1ItemId::LeadBar:
  case engine::TR1ItemId::LeadBarSprite:
    break;
  case engine::TR1ItemId::ShotgunAmmo:
  case engine::TR1ItemId::ShotgunAmmoSprite:
    totalItemCount = world.getPlayer().getInventory().getAmmo(engine::WeaponType::Shotgun).getClips() * 2;
    suffix = std::string{" "} + world.getPlayer().getInventory().getAmmo(engine::WeaponType::Shotgun).iconChar;
    break;
  case engine::TR1ItemId::MagnumAmmo:
  case engine::TR1ItemId::MagnumAmmoSprite:
    totalItemCount = world.getPlayer().getInventory().getAmmo(engine::WeaponType::Magnums).getClips() * 2;
    suffix = std::string{" "} + world.getPlayer().getInventory().getAmmo(engine::WeaponType::Magnums).iconChar;
    break;
  case engine::TR1ItemId::UziAmmo:
  case engine::TR1ItemId::UziAmmoSprite:
    totalItemCount = world.getPlayer().getInventory().getAmmo(engine::WeaponType::Uzis).getClips() * 2;
    suffix = std::string{" "} + world.getPlayer().getInventory().getAmmo(engine::WeaponType::Uzis).iconChar;
    break;
  case engine::TR1ItemId::Shotgun:
  case engine::TR1ItemId::ShotgunSprite:
    drawAmmo(world.getPlayer().getInventory().getAmmo(engine::WeaponType::Shotgun));
    return;
  case engine::TR1ItemId::Magnums:
  case engine::TR1ItemId::MagnumsSprite:
    drawAmmo(world.getPlayer().getInventory().getAmmo(engine::WeaponType::Magnums));
    return;
  case engine::TR1ItemId::Uzis:
  case engine::TR1ItemId::UzisSprite:
    drawAmmo(world.getPlayer().getInventory().getAmmo(engine::WeaponType::Uzis));
    return;
  case engine::TR1ItemId::SmallMedipack:
  case engine::TR1ItemId::SmallMedipackSprite:
  case engine::TR1ItemId::LargeMedipack:
  case engine::TR1ItemId::LargeMedipackSprite:
    world.getEngine().getPresenter().initHealthBarTimeout();
    world.getEngine().getPresenter().constructBars(
      ui, world.getWorldGeometry().getPalette(), world.getObjectManager(), false);
    break;
  default:
    break;
  }

  if(totalItemCount > 1)
  {
    const ui::Text text{ui::makeAmmoString(std::to_string(totalItemCount) + suffix)};
    text.draw(ui,
              world.getEngine().getPresenter().getTrFont(),
              {(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y - RingInfoYMargin - 2 * ui::FontHeight});
  }
}

void MenuDisplay::tick(engine::world::World& world)
{
  for(auto& menuObject : getCurrentRing().list)
  {
    m_currentState->handleObjectTick(world, *this, menuObject);
  }
  if(auto newState = m_currentState->tick(world, *this))
  {
    m_currentState = std::move(newState);
    m_currentState->begin(world);
  }
}

void MenuDisplay::constructUi(ui::Ui& ui, engine::world::World& world)
{
  m_currentState->constructUi(ui, world, *this);
  constructNavigationHints(ui, world);
}

void MenuDisplay::constructNavigationHints(ui::Ui& ui, const engine::world::World& world) const
{
  static constexpr int RingInfoXMargin = 20;
  if(currentRingIndex > 0)
  {
    m_upArrow.draw(ui, world.getEngine().getPresenter().getTrFont(), {RingInfoXMargin, RingInfoYMargin});
    m_upArrow.draw(ui,
                   world.getEngine().getPresenter().getTrFont(),
                   {ui.getSize().x - m_upArrow.getWidth() - RingInfoXMargin, RingInfoYMargin});
  }

  if(currentRingIndex + 1 < rings.size())
  {
    m_downArrow.draw(ui,
                     world.getEngine().getPresenter().getTrFont(),
                     {RingInfoXMargin, ui.getSize().y - RingInfoYMargin + ui::FontHeight});
    m_downArrow.draw(
      ui,
      world.getEngine().getPresenter().getTrFont(),
      {ui.getSize().x - m_downArrow.getWidth() - RingInfoXMargin, ui.getSize().y - RingInfoYMargin + ui::FontHeight});
  }

  if(rings.size() > 1)
  {
    const ui::Text title{getCurrentRing().title};
    title.draw(
      ui, world.getEngine().getPresenter().getTrFont(), {(ui.getSize().x - title.getWidth()) / 2, RingInfoYMargin});
  }
}

bool MenuDisplay::doOptions(engine::world::World& world, MenuObject& object)
{
  switch(object.type)
  {
  case engine::TR1ItemId::Puzzle4:
    break;
  case engine::TR1ItemId::Key1:
  case engine::TR1ItemId::Key2:
  case engine::TR1ItemId::Key3:
  case engine::TR1ItemId::Key4:
  case engine::TR1ItemId::Item148:
  case engine::TR1ItemId::Item149:
  case engine::TR1ItemId::ScionPieceCounter:
  case engine::TR1ItemId::SmallMedipack:
  case engine::TR1ItemId::Explosive:
  case engine::TR1ItemId::LargeMedipack:
  case engine::TR1ItemId::Puzzle1:
  case engine::TR1ItemId::Pistols:
  case engine::TR1ItemId::Shotgun:
  case engine::TR1ItemId::Magnums:
  case engine::TR1ItemId::Uzis:
    return true;
  case engine::TR1ItemId::PistolAmmo:
  case engine::TR1ItemId::ShotgunAmmo:
  case engine::TR1ItemId::MagnumAmmo:
  case engine::TR1ItemId::UziAmmo:
  case engine::TR1ItemId::Sunglasses:
  case engine::TR1ItemId::CassettePlayer:
  case engine::TR1ItemId::DirectionKeys:
  case engine::TR1ItemId::PassportOpening:
    break;
  case engine::TR1ItemId::Flashlight:
    BOOST_THROW_EXCEPTION(std::runtime_error("Gamma options are not implemented"));
  case engine::TR1ItemId::Compass:
    if(world.getEngine().getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return)
       || world.getEngine().getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
    {
      object.animDirection = 1_mframe;
      object.goalFrame = object.lastMeshAnimFrame - 1_mframe;
    }
    break;
  default:
    if(world.getEngine().getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return)
       || world.getEngine().getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
    {
      object.animDirection = -1_mframe;
      object.goalFrame = 0_mframe;
    }
    break;
  }

  return false;
}

std::vector<MenuObject> MenuDisplay::getOptionRingObjects(engine::world::World& world,
                                                          const bool withHomePolaroid) const
{
  std::vector objects{MenuObject{/* translators: TR charmap encoding */ _("Game"),
                                 engine::TR1ItemId::PassportClosed,
                                 30_mframe,
                                 14_mframe,
                                 25_deg,
                                 -24_deg,
                                 0_deg,
                                 384_len,
                                 0x13,
                                 0x13},
                      MenuObject{/* translators: TR charmap encoding */ _("Controls"),
                                 engine::TR1ItemId::DirectionKeys,
                                 1_mframe,
                                 0_mframe,
                                 30_deg,
                                 8_deg,
                                 0_deg,
                                 352_len},
                      MenuObject{/* translators: TR charmap encoding */ _("Sound"),
                                 engine::TR1ItemId::CassettePlayer,
                                 1_mframe,
                                 0_mframe,
                                 26_deg,
                                 -13_deg,
                                 0_deg,
                                 368_len},
                      MenuObject{/* translators: TR charmap encoding */ _("Detail Levels"),
                                 engine::TR1ItemId::Sunglasses,
                                 1_mframe,
                                 0_mframe,
                                 23_deg,
                                 -37_deg,
                                 0_deg,
                                 424_len}};
  if(withHomePolaroid)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Gym"),
                                    engine::TR1ItemId::LarasHomePolaroid,
                                    1_mframe,
                                    0_mframe,
                                    25_deg,
                                    -24_deg,
                                    0_deg,
                                    384_len});
  }

  std::erase_if(objects,
                [&world](const MenuObject& obj)
                {
                  return world.getWorldGeometry().findAnimatedModelForType(obj.type) == nullptr;
                });
  for(auto& object : objects)
    object.initModel(world, m_lightsBuffer);

  return objects;
}

std::vector<MenuObject> MenuDisplay::getMainRingObjects(engine::world::World& world) const
{
  std::vector objects{MenuObject{/* translators: TR charmap encoding */ _("Compass"),
                                 engine::TR1ItemId::Compass,
                                 25_mframe,
                                 10_mframe,
                                 24_deg,
                                 -45_deg,
                                 0_deg,
                                 456_len,
                                 0x05,
                                 0x05}};

  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Pistols) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Pistols"),
                                    engine::TR1ItemId::Pistols,
                                    12_mframe,
                                    11_mframe,
                                    18_deg,
                                    -21_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Shotgun) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Shotgun"),
                                    engine::TR1ItemId::Shotgun,
                                    13_mframe,
                                    12_mframe,
                                    18_deg,
                                    0_deg,
                                    -45_deg,
                                    296_len});
  }
  else if(world.getPlayer().getInventory().getAmmo(engine::WeaponType::Shotgun).getClips() > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Shotgun Cells"),
                                    engine::TR1ItemId::ShotgunAmmo,
                                    1_mframe,
                                    0_mframe,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }

  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Magnums) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Magnums"),
                                    engine::TR1ItemId::Magnums,
                                    12_mframe,
                                    11_mframe,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  else if(world.getPlayer().getInventory().getAmmo(engine::WeaponType::Magnums).getClips() > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Magnum Clips"),
                                    engine::TR1ItemId::MagnumAmmo,
                                    1_mframe,
                                    0_mframe,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }

  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Uzis) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Uzis"),
                                    engine::TR1ItemId::Uzis,
                                    13_mframe,
                                    12_mframe,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  else if(world.getPlayer().getInventory().getAmmo(engine::WeaponType::Uzis).getClips() > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Uzi Clips"),
                                    engine::TR1ItemId::UziAmmo,
                                    1_mframe,
                                    0_mframe,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Explosive) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Grenade"),
                                    engine::TR1ItemId::Explosive,
                                    15_mframe,
                                    14_mframe,
                                    28_deg,
                                    0_deg,
                                    0_deg,
                                    368_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::LargeMedipack) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Large Medi Pack"),
                                    engine::TR1ItemId::LargeMedipack,
                                    20_mframe,
                                    19_mframe,
                                    20_deg,
                                    -45_deg,
                                    -22.5_deg,
                                    352_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::SmallMedipack) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Small Medi Pack"),
                                    engine::TR1ItemId::SmallMedipack,
                                    26_mframe,
                                    25_mframe,
                                    22_deg,
                                    -40_deg,
                                    -22.5_deg,
                                    216_len});
  }

  std::erase_if(objects,
                [&world](const MenuObject& obj)
                {
                  return world.getWorldGeometry().findAnimatedModelForType(obj.type) == nullptr;
                });
  for(auto& object : objects)
    object.initModel(world, m_lightsBuffer);

  return objects;
}

std::vector<MenuObject> MenuDisplay::getKeysRingObjects(engine::world::World& world) const
{
  std::vector<MenuObject> objects{};

  if(world.getPlayer().getInventory().count(engine::TR1ItemId::LeadBar) > 0)
  {
    objects.emplace_back(
      MenuObject{"Lead Bar", engine::TR1ItemId::LeadBar, 1_mframe, 0_mframe, 20_deg, -45_deg, -22.5_deg, 352_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key1) > 0)
  {
    objects.emplace_back(
      MenuObject{"Key", engine::TR1ItemId::Key1, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key2) > 0)
  {
    objects.emplace_back(
      MenuObject{"Key", engine::TR1ItemId::Key2, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key3) > 0)
  {
    objects.emplace_back(
      MenuObject{"Key", engine::TR1ItemId::Key3, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key4) > 0)
  {
    objects.emplace_back(
      MenuObject{"Key", engine::TR1ItemId::Key4, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle4) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle4, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle3) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle3, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle2) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle2, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle1) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle1, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Item149) > 0)
  {
    objects.emplace_back(
      MenuObject{"Pickup", engine::TR1ItemId::Item149, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Item148) > 0)
  {
    objects.emplace_back(
      MenuObject{"Pickup", engine::TR1ItemId::Item148, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::ScionPieceCounter) > 0)
  {
    objects.emplace_back(
      MenuObject{"Scion", engine::TR1ItemId::ScionPieceCounter, 1_mframe, 0_mframe, 40_deg, -24_deg, 0_deg, 256_len});
  }

  std::erase_if(objects,
                [&world](const MenuObject& obj)
                {
                  return world.getWorldGeometry().findAnimatedModelForType(obj.type) == nullptr;
                });
  for(auto& object : objects)
    object.initModel(world, m_lightsBuffer);

  return objects;
}

MenuDisplay::MenuDisplay(const InventoryMode mode,
                         const SaveGamePageMode saveGamePageMode,
                         const bool allowPassportExit,
                         engine::world::World& world,
                         const glm::ivec2& viewport)
    : mode{mode}
    , allowMenuClose{mode != InventoryMode::TitleMode && mode != InventoryMode::DeathMode}
    , saveGamePageMode{saveGamePageMode}
    , allowPassportExit{allowPassportExit}
    , m_currentState{std::make_unique<InflateRingMenuState>(ringTransform, true)}
    , m_upArrow{ui::getSpriteSelector(ui::ArrowUpSprite)}
    , m_downArrow{ui::getSpriteSelector(ui::ArrowDownSprite)}
    , m_material{world.getEngine().getPresenter().getRenderSystem().getMaterialManager().getFlat(false, false, false)}
    , m_fb{gsl_lite::make_shared<render::pass::Framebuffer>(
        "menu-objects", m_material, render::scene::Translucency::Opaque, viewport)}
    , m_lightsBuffer{std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>(
        "lights-buffer",
        gl::api::BufferUsage::StaticDraw,
        engine::ShaderLight{glm::vec4{-500, 50, -5000, 0}, glm::vec4{1.0f}, 8192.0f})}
{
  if(mode == InventoryMode::GameMode)
  {
    rings.emplace_back(std::make_unique<MenuRing>(
      MenuRing::Type::Items, /* translators: TR charmap encoding */ _("ITEMS"), getKeysRingObjects(world)));
    if(rings.back()->list.empty())
    {
      rings.pop_back();
    }

    currentRingIndex = rings.size();
    rings.emplace_back(std::make_unique<MenuRing>(
      MenuRing::Type::Inventory, /* translators: TR charmap encoding */ _("INVENTORY"), getMainRingObjects(world)));
  }

  rings.emplace_back(std::make_unique<MenuRing>(
    MenuRing::Type::Options,
    mode == InventoryMode::DeathMode ? /* translators: TR charmap encoding */ _("GAME OVER")
                                     : /* translators: TR charmap encoding */ _("OPTION"),
    getOptionRingObjects(world,
                         mode == InventoryMode::TitleMode
                           && !world.getEngine().getScriptEngine().getGameflow().getLaraHome().empty())));

  m_currentState->begin(world);

  world.getCameraController().getCamera()->setFieldOfView(core::DefaultFov);
  // TODO fadeInInventory(mode != InventoryMode::TitleMode);
  world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr);
}

void MenuDisplay::setViewport(const glm::ivec2& viewport)
{
  if(m_fb->getOutput()->getTexture()->size() != viewport)
    m_fb = gsl_lite::make_shared<render::pass::Framebuffer>(
      "menu-objects", m_material, render::scene::Translucency::Opaque, viewport);
}

void MenuDisplay::renderObjectsToFramebuffer(engine::world::World& world)
{
  setViewport(world.getEngine().getPresenter().getRenderViewport());

  const auto& camera = world.getCameraController().getCamera();
  camera->setViewMatrix(ringTransform->getView());
  const auto resetFov = gsl_lite::finally(
    [oldFOV = camera->getFieldOfViewY(), &camera]
    {
      camera->setFieldOfView(oldFOV);
    });
  camera->setFieldOfView(core::DefaultFov);

  m_fb->render(
    [this, &world]
    {
      SOGLB_DEBUGGROUP("menu-objects");
      m_fb->getOutput()->getTexture()->clear({0, 0, 0, 0});
      m_fb->getDepthBuffer()->clear(gl::ScalarDepth32F{1.0f});
      world.getCameraController().getCamera()->setViewport(m_fb->getOutput()->getTexture()->size());

      auto itemAngle{0_deg};
      for(auto& menuObject : getCurrentRing().list)
      {
        menuObject.draw(world, *ringTransform, itemAngle);
        itemAngle += getCurrentRing().getAnglePerItem();
      }
    });
}

void MenuDisplay::renderFramebuffer(const engine::world::World& world)
{
  gl::RenderState::getWantedState().setViewport(world.getEngine().getPresenter().getDisplayViewport());
  m_fb->render();
}

MenuDisplay::~MenuDisplay() = default;
} // namespace menu