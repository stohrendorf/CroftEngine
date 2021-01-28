#include "menudisplay.h"

#include "core/pybindmodule.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/i18n.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "hid/inputhandler.h"
#include "menuring.h"
#include "menustates.h"
#include "ui/util.h"

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

void MenuDisplay::updateMenuObjectDescription(engine::World& world, const MenuObject& object)
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

  const size_t itemCount = world.getInventory().count(object.type);
  size_t totalItemCount = itemCount;
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
    totalItemCount = world.getObjectManager().getLara().shotgunAmmo.ammo / 6;
    suffix = " A";
    break;
  case engine::TR1ItemId::Magnums:
    totalItemCount = world.getObjectManager().getLara().revolverAmmo.ammo;
    suffix = " B";
    break;
  case engine::TR1ItemId::Uzis:
    totalItemCount = world.getObjectManager().getLara().uziAmmo.ammo;
    suffix = " C";
    break;
  case engine::TR1ItemId::SmallMedipack:
  case engine::TR1ItemId::LargeMedipack:
    world.getPresenter().setHealthBarTimeout(40_frame);
    world.getPresenter().drawBars(world.getPalette(), world.getObjectManager());
    break;
  default: break;
  }

  if(itemCount > 1)
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

void MenuDisplay::display(gl::Image<gl::SRGBA8>& img, engine::World& world)
{
  ringTransform->cameraPos.Z = 598_len + ringTransform->radius;

  core::Angle itemAngle{0_deg};
  world.getCameraController().getCamera()->setViewMatrix(ringTransform->getView());
  for(auto& menuObject : getCurrentRing().list)
  {
    MenuObject* object = &menuObject;
    m_currentState->handleObject(world, *this, *object);

    object->draw(world, *ringTransform, itemAngle);
    itemAngle += getCurrentRing().getAnglePerItem();
  }

  if(auto newState = m_currentState->onFrame(img, world, *this))
  {
    m_currentState = std::move(newState);
    m_currentState->begin();
  }

  for(const auto& txt : objectTexts)
    if(txt != nullptr)
      txt->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  if(result != MenuResult::None)
    world.getAudioEngine().setStreamVolume(streamGain);
}

void MenuDisplay::clearMenuObjectDescription()
{
  objectTexts[0].reset();
  objectTexts[1].reset();
}

bool MenuDisplay::doOptions(gl::Image<gl::SRGBA8>& /*img*/, engine::World& world, MenuObject& object)
{
  switch(object.type)
  {
  case engine::TR1ItemId::Puzzle4: break;
  case engine::TR1ItemId::Key1:
  case engine::TR1ItemId::Key2:
  case engine::TR1ItemId::Key3:
  case engine::TR1ItemId::Key4:
  case engine::TR1ItemId::Item148:
  case engine::TR1ItemId::Item149:
  case engine::TR1ItemId::ScionPiece5:
  case engine::TR1ItemId::SmallMedipack:
  case engine::TR1ItemId::Explosive:
  case engine::TR1ItemId::LargeMedipack:
  case engine::TR1ItemId::Puzzle1:
  case engine::TR1ItemId::Pistols:
  case engine::TR1ItemId::Shotgun:
  case engine::TR1ItemId::Magnums:
  case engine::TR1ItemId::Uzis: return true;
  case engine::TR1ItemId::PistolAmmo:
  case engine::TR1ItemId::ShotgunAmmo:
  case engine::TR1ItemId::MagnumAmmo:
  case engine::TR1ItemId::UziAmmo: break;
  case engine::TR1ItemId::Sunglasses: /* TODO doGraphicsOptions(); */ break;
  case engine::TR1ItemId::CassettePlayer: /* TODO doSoundOptions(); */ break;
  case engine::TR1ItemId::DirectionKeys: /* TODO doControlOptions(); */ break;
  case engine::TR1ItemId::Flashlight: /* TODO doGammaOptions(); */ break;
  case engine::TR1ItemId::PassportOpening: /* TODO doPassportOptions(); */ break;
  case engine::TR1ItemId::Compass:
    if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true)
       || world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
    {
      object.animDirection = 1_frame;
      object.goalFrame = object.lastMeshAnimFrame - 1_frame;
    }
    break;
  default:
    if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true)
       || world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
    {
      object.animDirection = -1_frame;
      object.goalFrame = 0_frame;
    }
    break;
  }

  return false;
}

std::vector<MenuObject> MenuDisplay::getOptionRingObjects(const engine::World& world, bool withHomePolaroid)
{
  std::vector objects{MenuObject{world.getEngine().i18n(engine::I18n::Game),
                                 engine::TR1ItemId::PassportClosed,
                                 30_frame,
                                 14_frame,
                                 25_deg,
                                 -24_deg,
                                 0_deg,
                                 384_len,
                                 0x13,
                                 0x13},
                      MenuObject{world.getEngine().i18n(engine::I18n::Controls),
                                 engine::TR1ItemId::DirectionKeys,
                                 1_frame,
                                 0_frame,
                                 30_deg,
                                 8_deg,
                                 0_deg,
                                 352_len},
                      MenuObject{world.getEngine().i18n(engine::I18n::Sound),
                                 engine::TR1ItemId::CassettePlayer,
                                 1_frame,
                                 0_frame,
                                 26_deg,
                                 -13_deg,
                                 0_deg,
                                 368_len},
                      MenuObject{world.getEngine().i18n(engine::I18n::DetailLevels),
                                 engine::TR1ItemId::Sunglasses,
                                 1_frame,
                                 0_frame,
                                 23_deg,
                                 -37_deg,
                                 0_deg,
                                 424_len}};
  if(withHomePolaroid)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::LarasHome),
                                    engine::TR1ItemId::LarasHomePolaroid,
                                    1_frame,
                                    0_frame,
                                    25_deg,
                                    -24_deg,
                                    0_deg,
                                    384_len});
  }

  for(auto& object : objects)
    object.initModel(world);

  return objects;
}

std::vector<MenuObject> MenuDisplay::getMainRingObjects(const engine::World& world)
{
  std::vector objects{MenuObject{world.getEngine().i18n(engine::I18n::Compass),
                                 engine::TR1ItemId::Compass,
                                 25_frame,
                                 10_frame,
                                 24_deg,
                                 -45_deg,
                                 0_deg,
                                 456_len,
                                 0x05,
                                 0x05}};

  if(world.getInventory().count(engine::TR1ItemId::Pistols) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::Controls),
                                    engine::TR1ItemId::Pistols,
                                    12_frame,
                                    11_frame,
                                    18_deg,
                                    -21_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Shotgun) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::Shotgun),
                                    engine::TR1ItemId::Shotgun,
                                    13_frame,
                                    12_frame,
                                    18_deg,
                                    0_deg,
                                    -45_deg,
                                    296_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Magnums) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::Magnums),
                                    engine::TR1ItemId::Magnums,
                                    12_frame,
                                    11_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Uzis) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::Uzis),
                                    engine::TR1ItemId::Uzis,
                                    13_frame,
                                    12_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Explosive) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::Grenade),
                                    engine::TR1ItemId::Explosive,
                                    15_frame,
                                    14_frame,
                                    28_deg,
                                    0_deg,
                                    0_deg,
                                    368_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::LargeMedipack) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::LargeMediPack),
                                    engine::TR1ItemId::LargeMedipack,
                                    20_frame,
                                    19_frame,
                                    20_deg,
                                    -45_deg,
                                    -22.5_deg,
                                    352_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::SmallMedipack) > 0)
  {
    objects.emplace_back(MenuObject{world.getEngine().i18n(engine::I18n::SmallMediPack),
                                    engine::TR1ItemId::SmallMedipack,
                                    26_frame,
                                    25_frame,
                                    22_deg,
                                    -40_deg,
                                    -22.5_deg,
                                    216_len});
  }

  for(auto& object : objects)
    object.initModel(world);

  return objects;
}

std::vector<MenuObject> MenuDisplay::getKeysRingObjects(const engine::World& world)
{
  std::vector<MenuObject> objects{};

  if(world.getInventory().count(engine::TR1ItemId::LeadBar) > 0)
  {
    objects.emplace_back(
      MenuObject{"Lead Bar", engine::TR1ItemId::LeadBar, 1_frame, 0_frame, 20_deg, -45_deg, -22.5_deg, 352_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Key1) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key1, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Key2) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key2, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Key3) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key3, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Key4) > 0)
  {
    objects.emplace_back(MenuObject{"Key", engine::TR1ItemId::Key4, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Puzzle4) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle4, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Puzzle3) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle3, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Puzzle2) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle2, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Puzzle1) > 0)
  {
    objects.emplace_back(
      MenuObject{"Puzzle", engine::TR1ItemId::Puzzle1, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Item149) > 0)
  {
    objects.emplace_back(
      MenuObject{"Pickup", engine::TR1ItemId::Item149, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }
  if(world.getInventory().count(engine::TR1ItemId::Item148) > 0)
  {
    objects.emplace_back(
      MenuObject{"Pickup", engine::TR1ItemId::Item148, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len});
  }

  for(auto& object : objects)
    object.initModel(world);

  return objects;
}

MenuDisplay::MenuDisplay(InventoryMode mode, engine::World& world)
    : mode{mode}
    , streamGain{world.getPresenter().getSoundEngine()->getSoLoud().getGlobalVolume()}
    , m_currentState{std::make_unique<InflateRingMenuState>(ringTransform)}
{
  m_currentState->begin();

  if(mode == InventoryMode::KeysMode || mode == InventoryMode::GameMode)
    rings.emplace_back(std::make_unique<MenuRing>(
      MenuRing::Type::Inventory, world.getEngine().i18n(engine::I18n::Inventory), getMainRingObjects(world)));

  if(mode != InventoryMode::KeysMode)
  {
    rings.emplace_back(std::make_unique<MenuRing>(
      MenuRing::Type::Options,
      world.getEngine().i18n(mode == InventoryMode::DeathMode ? engine::I18n::GameOver : engine::I18n::Option),
      getOptionRingObjects(world, mode == InventoryMode::TitleMode)));
  }

  if(mode == InventoryMode::KeysMode || mode == InventoryMode::GameMode)
  {
    rings.emplace_back(std::make_unique<MenuRing>(
      MenuRing::Type::Items, world.getEngine().i18n(engine::I18n::Items), getKeysRingObjects(world)));
    if(rings.back()->list.empty())
    {
      if(mode == InventoryMode::KeysMode)
      {
        return;
      }

      rings.pop_back();
    }
  }

  Ensures(!rings.empty());

  world.getCameraController().getCamera()->setFieldOfView(core::toRad(80_deg));
  // TODO fadeInInventory(mode != InventoryMode::TitleMode);
  if(mode != InventoryMode::TitleMode)
  {
    world.getAudioEngine().setStreamVolume(0);
  }
  world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr);
}

MenuDisplay::~MenuDisplay() = default;
} // namespace menu
