#include "menudisplay.h"

#include "core/i18n.h"
#include "core/pybindmodule.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/objects/laraobject.h"
#include "engine/player.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"
#include "inflateringmenustate.h"
#include "menuring.h"
#include "render/scene/camera.h"

namespace menu
{
void MenuDisplay::updateRingTitle()
{
  if(rings.size() == 1)
    return;

  if(objectTexts[2] == nullptr)
  {
    objectTexts[2] = std::make_unique<ui::Label>(glm::ivec2{0, 26}, getCurrentRing().title);
    objectTexts[2]->alignX = ui::Label::Alignment::Center;
  }
  else
  {
    objectTexts[2]->text = getCurrentRing().title;
  }

  if(currentRingIndex > 0)
  {
    if(objectTexts[3] == nullptr)
    {
      objectTexts[3] = std::make_unique<ui::Label>(glm::ivec2{20, 28}, "[");
      objectTexts[3]->alignX = ui::Label::Alignment::Left;
      objectTexts[4] = std::make_unique<ui::Label>(glm::ivec2{-20, 28}, "[");
      objectTexts[4]->alignX = ui::Label::Alignment::Right;
    }
  }
  else
  {
    objectTexts[3].reset();
    objectTexts[4].reset();
  }

  if(currentRingIndex + 1 < rings.size())
  {
    if(objectTexts[5] == nullptr)
    {
      objectTexts[5] = std::make_unique<ui::Label>(glm::ivec2{20, -15}, "]");
      objectTexts[5]->alignX = ui::Label::Alignment::Left;
      objectTexts[5]->alignY = ui::Label::Alignment::Bottom;
      objectTexts[6] = std::make_unique<ui::Label>(glm::ivec2{-20, -15}, "]");
      objectTexts[6]->alignX = ui::Label::Alignment::Right;
      objectTexts[6]->alignY = ui::Label::Alignment::Bottom;
    }
  }
  else
  {
    objectTexts[5].reset();
    objectTexts[6].reset();
  }
}

void MenuDisplay::updateMenuObjectDescription(ui::Ui& ui, engine::world::World& world, const MenuObject& object)
{
  if(objectTexts[0] == nullptr)
  {
    if(const auto objectName = world.getItemTitle(object.type))
    {
      objectTexts[0] = std::make_unique<ui::Label>(glm::ivec2{0, -16}, objectName.value());
    }

    if(objectTexts[0] == nullptr)
      objectTexts[0] = std::make_unique<ui::Label>(glm::ivec2{0, -16}, object.name);

    objectTexts[0]->alignX = ui::Label::Alignment::Center;
    objectTexts[0]->alignY = ui::Label::Alignment::Bottom;
  }

  size_t totalItemCount = world.getPlayer().getInventory().count(object.type);
  std::string suffix;

  switch(object.type)
  {
  case engine::TR1ItemId::Puzzle1:
  case engine::TR1ItemId::Puzzle2:
  case engine::TR1ItemId::Puzzle3:
  case engine::TR1ItemId::Puzzle4:
  case engine::TR1ItemId::Key1:
  case engine::TR1ItemId::Key2:
  case engine::TR1ItemId::Key3:
  case engine::TR1ItemId::Key4:
  case engine::TR1ItemId::Item148:
  case engine::TR1ItemId::Item149:
  case engine::TR1ItemId::ScionPiece5:
  case engine::TR1ItemId::LeadBar: break;
  case engine::TR1ItemId::MagnumAmmo:
  case engine::TR1ItemId::UziAmmo:
  case engine::TR1ItemId::ShotgunAmmo: totalItemCount *= 2; break;
  case engine::TR1ItemId::Shotgun:
    totalItemCount = world.getPlayer().getInventory().getAmmo(engine::WeaponType::Shotgun)->ammo / 6;
    suffix = " A";
    break;
  case engine::TR1ItemId::Magnums:
    totalItemCount = world.getPlayer().getInventory().getAmmo(engine::WeaponType::Magnums)->ammo;
    suffix = " B";
    break;
  case engine::TR1ItemId::Uzis:
    totalItemCount = world.getPlayer().getInventory().getAmmo(engine::WeaponType::Uzis)->ammo;
    suffix = " C";
    break;
  case engine::TR1ItemId::SmallMedipack:
  case engine::TR1ItemId::LargeMedipack:
    world.getPresenter().setHealthBarTimeout(40_frame);
    world.getPresenter().drawBars(ui, world.getPalette(), world.getObjectManager());
    break;
  default: break;
  }

  if(totalItemCount > 1)
  {
    if(objectTexts[1] == nullptr)
      objectTexts[1]
        = std::make_unique<ui::Label>(glm::ivec2{64, -56}, ui::makeAmmoString(std::to_string(totalItemCount) + suffix));
    objectTexts[1]->alignX = ui::Label::Alignment::Center;
    objectTexts[1]->alignY = ui::Label::Alignment::Bottom;
  }
  else
  {
    objectTexts[1].reset();
  }
}

void MenuDisplay::display(ui::Ui& ui, engine::world::World& world)
{
  core::Angle itemAngle{0_deg};
  world.getCameraController().getCamera()->setViewMatrix(ringTransform->getView());
  for(auto& menuObject : getCurrentRing().list)
  {
    MenuObject* object = &menuObject;
    m_currentState->handleObject(ui, world, *this, *object);

    object->draw(world, *ringTransform, itemAngle);
    itemAngle += getCurrentRing().getAnglePerItem();
  }

  if(auto newState = m_currentState->onFrame(ui, world, *this))
  {
    m_currentState = std::move(newState);
    m_currentState->begin(world);
  }

  for(const auto& txt : objectTexts)
    if(txt != nullptr)
      txt->draw(ui, world.getPresenter().getTrFont(), world.getPresenter().getViewport());

  if(result != MenuResult::None)
    world.getAudioEngine().fadeStreamVolume(streamVolume);
}

void MenuDisplay::clearMenuObjectDescription()
{
  objectTexts[0].reset();
  objectTexts[1].reset();
}

bool MenuDisplay::doOptions(engine::world::World& world, MenuObject& object)
{
  switch(object.type)
  {
  case engine::TR1ItemId::Puzzle4: break;
  case engine::TR1ItemId::Key1: [[fallthrough]];
  case engine::TR1ItemId::Key2: [[fallthrough]];
  case engine::TR1ItemId::Key3: [[fallthrough]];
  case engine::TR1ItemId::Key4: [[fallthrough]];
  case engine::TR1ItemId::Item148: [[fallthrough]];
  case engine::TR1ItemId::Item149: [[fallthrough]];
  case engine::TR1ItemId::ScionPiece5: [[fallthrough]];
  case engine::TR1ItemId::SmallMedipack: [[fallthrough]];
  case engine::TR1ItemId::Explosive: [[fallthrough]];
  case engine::TR1ItemId::LargeMedipack: [[fallthrough]];
  case engine::TR1ItemId::Puzzle1: [[fallthrough]];
  case engine::TR1ItemId::Pistols: [[fallthrough]];
  case engine::TR1ItemId::Shotgun: [[fallthrough]];
  case engine::TR1ItemId::Magnums: [[fallthrough]];
  case engine::TR1ItemId::Uzis: return true;
  case engine::TR1ItemId::PistolAmmo: [[fallthrough]];
  case engine::TR1ItemId::ShotgunAmmo: [[fallthrough]];
  case engine::TR1ItemId::MagnumAmmo: [[fallthrough]];
  case engine::TR1ItemId::UziAmmo: [[fallthrough]];
  case engine::TR1ItemId::Sunglasses: break;
  case engine::TR1ItemId::CassettePlayer: /* TODO doSoundOptions(); */ break;
  case engine::TR1ItemId::DirectionKeys: /* TODO doControlOptions(); */ break;
  case engine::TR1ItemId::Flashlight: /* TODO doGammaOptions(); */ break;
  case engine::TR1ItemId::PassportOpening: /* TODO doPassportOptions(); */ break;
  case engine::TR1ItemId::Compass:
    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu)
       || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
    {
      object.animDirection = 1_frame;
      object.goalFrame = object.lastMeshAnimFrame - 1_frame;
    }
    break;
  default:
    if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action)
       || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
    {
      object.animDirection = -1_frame;
      object.goalFrame = 0_frame;
    }
    break;
  }

  return false;
}

std::vector<MenuObject> MenuDisplay::getOptionRingObjects(const engine::world::World& world, bool withHomePolaroid)
{
  std::vector objects{MenuObject{/* translators: TR charmap encoding */ _("Game"),
                                 engine::TR1ItemId::PassportClosed,
                                 30_frame,
                                 14_frame,
                                 25_deg,
                                 -24_deg,
                                 0_deg,
                                 384_len,
                                 0x13,
                                 0x13},
                      MenuObject{/* translators: TR charmap encoding */ _("Controls"),
                                 engine::TR1ItemId::DirectionKeys,
                                 1_frame,
                                 0_frame,
                                 30_deg,
                                 8_deg,
                                 0_deg,
                                 352_len},
                      MenuObject{/* translators: TR charmap encoding */ _("Sound"),
                                 engine::TR1ItemId::CassettePlayer,
                                 1_frame,
                                 0_frame,
                                 26_deg,
                                 -13_deg,
                                 0_deg,
                                 368_len},
                      MenuObject{/* translators: TR charmap encoding */ _("Detail Levels"),
                                 engine::TR1ItemId::Sunglasses,
                                 1_frame,
                                 0_frame,
                                 23_deg,
                                 -37_deg,
                                 0_deg,
                                 424_len}};
  if(withHomePolaroid)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Gym"),
                                    engine::TR1ItemId::LarasHomePolaroid,
                                    1_frame,
                                    0_frame,
                                    25_deg,
                                    -24_deg,
                                    0_deg,
                                    384_len});
  }

  objects.erase(std::remove_if(objects.begin(),
                               objects.end(),
                               [&world](const MenuObject& obj)
                               { return world.findAnimatedModelForType(obj.type) == nullptr; }),
                objects.end());
  for(auto& object : objects)
    object.initModel(world);

  return objects;
}

std::vector<MenuObject> MenuDisplay::getMainRingObjects(const engine::world::World& world)
{
  std::vector objects{MenuObject{/* translators: TR charmap encoding */ _("Compass"),
                                 engine::TR1ItemId::Compass,
                                 25_frame,
                                 10_frame,
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
                                    12_frame,
                                    11_frame,
                                    18_deg,
                                    -21_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Shotgun) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Shotgun"),
                                    engine::TR1ItemId::Shotgun,
                                    13_frame,
                                    12_frame,
                                    18_deg,
                                    0_deg,
                                    -45_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Magnums) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Magnums"),
                                    engine::TR1ItemId::Magnums,
                                    12_frame,
                                    11_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Uzis) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Uzis"),
                                    engine::TR1ItemId::Uzis,
                                    13_frame,
                                    12_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::ShotgunAmmo) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Shotgun Cells"),
                                    engine::TR1ItemId::ShotgunAmmo,
                                    1_frame,
                                    0_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::MagnumAmmo) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Magnum Clips"),
                                    engine::TR1ItemId::MagnumAmmo,
                                    1_frame,
                                    0_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::UziAmmo) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Uzi Clips"),
                                    engine::TR1ItemId::UziAmmo,
                                    1_frame,
                                    0_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Explosive) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Grenade"),
                                    engine::TR1ItemId::Explosive,
                                    15_frame,
                                    14_frame,
                                    28_deg,
                                    0_deg,
                                    0_deg,
                                    368_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::LargeMedipack) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Large Medi Pack"),
                                    engine::TR1ItemId::LargeMedipack,
                                    20_frame,
                                    19_frame,
                                    20_deg,
                                    -45_deg,
                                    -22.5_deg,
                                    352_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::SmallMedipack) > 0)
  {
    objects.emplace_back(MenuObject{/* translators: TR charmap encoding */ _("Small Medi Pack"),
                                    engine::TR1ItemId::SmallMedipack,
                                    26_frame,
                                    25_frame,
                                    22_deg,
                                    -40_deg,
                                    -22.5_deg,
                                    216_len});
  }

  objects.erase(std::remove_if(objects.begin(),
                               objects.end(),
                               [&world](const MenuObject& obj)
                               { return world.findAnimatedModelForType(obj.type) == nullptr; }),
                objects.end());
  for(auto& object : objects)
    object.initModel(world);

  return objects;
}

std::vector<MenuObject> MenuDisplay::getKeysRingObjects(const engine::world::World& world)
{
  std::vector<MenuObject> objects{};

  if(world.getPlayer().getInventory().count(engine::TR1ItemId::LeadBar) > 0)
  {
    objects.emplace_back(
      MenuObject{"Lead Bar", engine::TR1ItemId::LeadBar, 1_frame, 0_frame, 20_deg, -45_deg, -22.5_deg, 352_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key1) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key1, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key2) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key2, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key3) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key3, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Key4) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key4, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle4) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle4, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle3) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle3, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle2) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle2, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Puzzle1) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle1, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Item149) > 0)
  {
    objects.emplace_back(
      MenuObject{"Pickup", engine::TR1ItemId::Item149, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::Item148) > 0)
  {
    objects.emplace_back(
      MenuObject{"Pickup", engine::TR1ItemId::Item148, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getPlayer().getInventory().count(engine::TR1ItemId::ScionPiece5) > 0)
  {
    objects.emplace_back(
      MenuObject{"Scion", engine::TR1ItemId::ScionPiece5, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }

  objects.erase(std::remove_if(objects.begin(),
                               objects.end(),
                               [&world](const MenuObject& obj)
                               { return world.findAnimatedModelForType(obj.type) == nullptr; }),
                objects.end());
  for(auto& object : objects)
    object.initModel(world);

  return objects;
}

MenuDisplay::MenuDisplay(InventoryMode mode, engine::world::World& world)
    : mode{mode}
    , streamVolume{world.getPresenter().getSoundEngine()->getSoLoud().getGlobalVolume()}
    , allowMenuClose{mode != InventoryMode::TitleMode && mode != InventoryMode::DeathMode}
    , m_currentState{std::make_unique<InflateRingMenuState>(ringTransform, true)}
{
  if(mode == InventoryMode::GameMode)
  {
    rings.emplace_back(std::make_unique<MenuRing>(
      MenuRing::Type::Items, /* translators: TR charmap encoding */ _("ITEMS"), getKeysRingObjects(world)));
    if(rings.back()->list.empty())
    {
      rings.pop_back();
    }
  }

  if(mode == InventoryMode::GameMode)
  {
    currentRingIndex = rings.size();
    rings.emplace_back(

      std::make_unique<MenuRing>(
        MenuRing::Type::Inventory, /* translators: TR charmap encoding */ _("INVENTORY"), getMainRingObjects(world)));
  }

  rings.emplace_back(std::make_unique<MenuRing>(MenuRing::Type::Options,
                                                mode == InventoryMode::DeathMode
                                                  ? /* translators: TR charmap encoding */ _("GAME OVER")
                                                  : /* translators: TR charmap encoding */ _("OPTION"),
                                                getOptionRingObjects(world, mode == InventoryMode::TitleMode)));

  m_currentState->begin(world);

  world.getCameraController().getCamera()->setFieldOfView(engine::Presenter::DefaultFov);
  // TODO fadeInInventory(mode != InventoryMode::TitleMode);
  if(mode != InventoryMode::TitleMode)
  {
    world.getAudioEngine().fadeStreamVolume(0.1f);
  }
  world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr);
}

MenuDisplay::~MenuDisplay() = default;
} // namespace menu
