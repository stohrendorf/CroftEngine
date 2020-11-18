#include "menudisplay.h"

#include "core/pybindmodule.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "hid/inputhandler.h"
#include "menuring.h"
#include "menustates.h"
#include "ui/util.h"

namespace menu
{
namespace
{
std::string makeAmmoString(const std::string& str)
{
  std::string result;
  for(const char c : str)
  {
    if(c == ' ')
    {
      result += c;
      continue;
    }

    if(c < 'A')
    {
      result += static_cast<char>(c - '0' + 1);
    }
    else
    {
      result += static_cast<char>(c - 'A' + 12);
    }
  }
  return result;
}
} // namespace

void MenuDisplay::updateRingTitle()
{
  if(rings.size() == 1)
    return;

  if(objectTexts[2] == nullptr)
  {
    objectTexts[2] = std::make_unique<ui::Label>(0, 26, getCurrentRing().title);
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
      objectTexts[3] = std::make_unique<ui::Label>(20, 28, "[");
      objectTexts[3]->alignX = ui::Label::Alignment::Left;
      objectTexts[4] = std::make_unique<ui::Label>(-20, 28, "[");
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
      objectTexts[5] = std::make_unique<ui::Label>(20, -15, "]");
      objectTexts[5]->alignX = ui::Label::Alignment::Left;
      objectTexts[5]->alignY = ui::Label::Alignment::Bottom;
      objectTexts[6] = std::make_unique<ui::Label>(-20, -15, "]");
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

void MenuDisplay::updateMenuObjectDescription(engine::Engine& engine, const MenuObject& object)
{
  if(objectTexts[0] == nullptr)
  {
    if(const auto names = core::get<pybind11::dict>(engine.getLevelInfo(), "names"))
    {
      if(const auto localNames = core::get<pybind11::dict>(names.value(), engine.getLanguage()))
      {
        if(const auto objectName = core::get<std::string>(localNames.value(), object.type))
        {
          objectTexts[0] = std::make_unique<ui::Label>(0, -16, objectName.value());
        }
      }
    }

    if(objectTexts[0] == nullptr)
      objectTexts[0] = std::make_unique<ui::Label>(0, -16, object.name);

    objectTexts[0]->alignX = ui::Label::Alignment::Center;
    objectTexts[0]->alignY = ui::Label::Alignment::Bottom;
  }

  const size_t itemCount = engine.getInventory().count(object.type);
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
    totalItemCount = engine.getObjectManager().getLara().shotgunAmmo.ammo / 6;
    suffix = " A";
    break;
  case engine::TR1ItemId::Magnums:
    totalItemCount = engine.getObjectManager().getLara().revolverAmmo.ammo;
    suffix = " B";
    break;
  case engine::TR1ItemId::Uzis:
    totalItemCount = engine.getObjectManager().getLara().uziAmmo.ammo;
    suffix = " C";
    break;
  case engine::TR1ItemId::SmallMedipack:
  case engine::TR1ItemId::LargeMedipack:
    engine.getPresenter().setHealthBarTimeout(40_frame);
    engine.getPresenter().drawBars(engine.getPalette(), engine.getObjectManager());
    break;
  default: break;
  }

  if(objectTexts[1] == nullptr && itemCount > 1)
  {
    objectTexts[1] = std::make_unique<ui::Label>(64, -56, makeAmmoString(std::to_string(totalItemCount) + suffix));
    objectTexts[1]->alignX = ui::Label::Alignment::Center;
    objectTexts[1]->alignY = ui::Label::Alignment::Bottom;
  }
  else
  {
    objectTexts[1].reset();
  }
}

void MenuDisplay::display(gl::Image<gl::SRGBA8>& img, engine::Engine& engine)
{
  ringTransform->cameraPos.Z = 598_len + ringTransform->radius;

  core::Angle itemAngle{0_deg};
  engine.getCameraController().getCamera()->setViewMatrix(ringTransform->getView());
  for(auto& menuObject : getCurrentRing().list)
  {
    MenuObject* object = &menuObject;
    m_currentState->handleObject(engine, *this, *object);

    object->draw(engine, *ringTransform, itemAngle);
    itemAngle += getCurrentRing().getAnglePerItem();
  }

  if(auto newState = m_currentState->onFrame(img, engine, *this))
  {
    m_currentState = std::move(newState);
    m_currentState->begin();
  }

  for(const auto& txt : objectTexts)
    if(txt != nullptr)
      txt->draw(engine.getPresenter().getTrFont(), img, engine.getPalette());
}

void MenuDisplay::clearMenuObjectDescription()
{
  objectTexts[0].reset();
  objectTexts[1].reset();
}

bool MenuDisplay::doOptions(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& engine, MenuObject& object)
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
    if(engine.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true)
       || engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
    {
      object.animDirection = 1_frame;
      object.goalFrame = object.lastMeshAnimFrame - 1_frame;
    }
    break;
  default:
    if(engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true)
       || engine.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
    {
      object.animDirection = -1_frame;
      object.goalFrame = 0_frame;
    }
    break;
  }

  return false;
}

std::vector<MenuObject> MenuDisplay::getOptionRingObjects(bool withHomePolaroid)
{
  std::vector<MenuObject> objects{
    MenuObject{
      "Game", engine::TR1ItemId::PassportClosed, 30_frame, 14_frame, 25_deg, -24_deg, 0_deg, 384_len, 0x13, 0x13},
    MenuObject{"Controls",
               engine::TR1ItemId::DirectionKeys,
               1_frame,
               0_frame,
               30_deg,
               8_deg,
               0_deg,
               352_len,
               0xffffffff,
               0xffffffff},
    MenuObject{"Sound",
               engine::TR1ItemId::CassettePlayer,
               1_frame,
               0_frame,
               26_deg,
               -13_deg,
               0_deg,
               368_len,
               0xffffffff,
               0xffffffff},
    MenuObject{"Detail Levels",
               engine::TR1ItemId::Sunglasses,
               1_frame,
               0_frame,
               23_deg,
               -37_deg,
               0_deg,
               424_len,
               0xffffffff,
               0xffffffff}};
  if(withHomePolaroid)
  {
    objects.emplace_back(MenuObject{"Lara's home",
                                    engine::TR1ItemId::LarasHomePolaroid,
                                    1_frame,
                                    0_frame,
                                    25_deg,
                                    -24_deg,
                                    0_deg,
                                    384_len,
                                    0xffffffff,
                                    0xffffffff});
  }

  return objects;
}

std::vector<MenuObject> MenuDisplay::getMainRingObjects(const engine::Engine& engine)
{
  std::vector<MenuObject> objects{
    MenuObject{"Compass", engine::TR1ItemId::Compass, 25_frame, 10_frame, 24_deg, -45_deg, 0_deg, 456_len, 0x05, 0x05}};

  if(engine.getInventory().count(engine::TR1ItemId::Pistols) > 0)
  {
    objects.emplace_back(MenuObject{"Pistols",
                                    engine::TR1ItemId::Pistols,
                                    12_frame,
                                    11_frame,
                                    18_deg,
                                    -21_deg,
                                    0_deg,
                                    296_len,
                                    0xffffffff,
                                    0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Shotgun) > 0)
  {
    objects.emplace_back(MenuObject{"Shotgun",
                                    engine::TR1ItemId::Shotgun,
                                    13_frame,
                                    12_frame,
                                    18_deg,
                                    0_deg,
                                    -45_deg,
                                    296_len,
                                    0xffffffff,
                                    0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Magnums) > 0)
  {
    objects.emplace_back(MenuObject{"Magnums",
                                    engine::TR1ItemId::Magnums,
                                    12_frame,
                                    11_frame,
                                    18_deg,
                                    -20_deg,
                                    0_deg,
                                    296_len,
                                    0xffffffff,
                                    0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Uzis) > 0)
  {
    objects.emplace_back(MenuObject{
      "Uzis", engine::TR1ItemId::Uzis, 13_frame, 12_frame, 18_deg, -20_deg, 0_deg, 296_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Explosive) > 0)
  {
    objects.emplace_back(MenuObject{"Grenade",
                                    engine::TR1ItemId::Explosive,
                                    15_frame,
                                    14_frame,
                                    28_deg,
                                    0_deg,
                                    0_deg,
                                    368_len,
                                    0xffffffff,
                                    0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::LargeMedipack) > 0)
  {
    objects.emplace_back(MenuObject{"Large Medi Pack",
                                    engine::TR1ItemId::LargeMedipack,
                                    20_frame,
                                    19_frame,
                                    20_deg,
                                    -45_deg,
                                    -22.5_deg,
                                    352_len,
                                    0xffffffff,
                                    0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::SmallMedipack) > 0)
  {
    objects.emplace_back(MenuObject{"Small Medi Pack",
                                    engine::TR1ItemId::SmallMedipack,
                                    26_frame,
                                    25_frame,
                                    22_deg,
                                    -40_deg,
                                    -22.5_deg,
                                    216_len,
                                    0xffffffff,
                                    0xffffffff});
  }

  return objects;
}

std::vector<MenuObject> MenuDisplay::getKeysRingObjects(const engine::Engine& engine)
{
  std::vector<MenuObject> objects{};

  if(engine.getInventory().count(engine::TR1ItemId::LeadBar) > 0)
  {
    objects.emplace_back(MenuObject{"Lead Bar",
                                    engine::TR1ItemId::LeadBar,
                                    1_frame,
                                    0_frame,
                                    20_deg,
                                    -45_deg,
                                    -22.5_deg,
                                    352_len,
                                    0xffffffff,
                                    0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Key1) > 0)
  {
    objects.emplace_back(MenuObject{
      "Key", engine::TR1ItemId::Key1, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Key2) > 0)
  {
    objects.emplace_back(MenuObject{
      "Key", engine::TR1ItemId::Key2, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Key3) > 0)
  {
    objects.emplace_back(MenuObject{
      "Key", engine::TR1ItemId::Key3, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Key4) > 0)
  {
    objects.emplace_back(MenuObject{
      "Key", engine::TR1ItemId::Key4, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Puzzle4) > 0)
  {
    objects.emplace_back(MenuObject{
      "Puzzle", engine::TR1ItemId::Puzzle4, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Puzzle3) > 0)
  {
    objects.emplace_back(MenuObject{
      "Puzzle", engine::TR1ItemId::Puzzle3, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Puzzle2) > 0)
  {
    objects.emplace_back(MenuObject{
      "Puzzle", engine::TR1ItemId::Puzzle2, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Puzzle1) > 0)
  {
    objects.emplace_back(MenuObject{
      "Puzzle", engine::TR1ItemId::Puzzle1, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Item149) > 0)
  {
    objects.emplace_back(MenuObject{
      "Pickup", engine::TR1ItemId::Item149, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }
  if(engine.getInventory().count(engine::TR1ItemId::Item148) > 0)
  {
    objects.emplace_back(MenuObject{
      "Pickup", engine::TR1ItemId::Item148, 1_frame, 0_frame, 40_deg, -24_deg, 0_deg, 256_len, 0xffffffff, 0xffffffff});
  }

  return objects;
}

bool MenuDisplay::init(engine::Engine& engine)
{
  inventoryChosen.reset();
  currentRingIndex = 0;

  if(mode == InventoryMode::KeysMode || mode == InventoryMode::GameMode)
    rings.emplace_back(std::make_unique<MenuRing>(MenuRing::Type::Inventory, "INVENTORY", getMainRingObjects(engine)));

  if(mode != InventoryMode::KeysMode)
  {
    rings.emplace_back(std::make_unique<MenuRing>(MenuRing::Type::Options,
                                                  mode == InventoryMode::DeathMode ? "GAME OVER" : "OPTION",
                                                  getOptionRingObjects(mode == InventoryMode::TitleMode)));
  }

  if(mode == InventoryMode::KeysMode || mode == InventoryMode::GameMode)
  {
    rings.emplace_back(std::make_unique<MenuRing>(MenuRing::Type::Items, "ITEMS", getKeysRingObjects(engine)));
    if(rings.back()->list.empty())
    {
      if(mode == InventoryMode::KeysMode)
        return false;

      rings.pop_back();
    }
  }

  Ensures(!rings.empty());

  engine.getCameraController().getCamera()->setFieldOfView(core::toRad(80_deg));
  passportPage = 0;
  selectedOption = 0;
  // TODO fadeInInventory(mode != InventoryMode::TitleMode);
  // TODO stopAmbientSamples();
  // TODO soundStopAllSamples();
  if(mode != InventoryMode::TitleMode)
  {
    // TODO setStreamVolume(0);
  }
  m_currentState = std::make_unique<InflateRingMenuState>(ringTransform);
  m_currentState->begin();
  engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr);
  passOpen = false;
  return true;
}

void MenuDisplay::finalize(engine::Engine& engine)
{
  //RemoveInventoryText();
  //Inventory_Displaying = 0;
  if(!inventoryChosen.has_value())
  {
    if(!allowMenuClose)
    {
      return;
    }
    if(musicVolume != 0)
    {
      // TODO setStreamVolume(25 * musicVolume + 5);
    }
    return;
  }

  if(inventoryChosen.value() == engine::TR1ItemId::PassportOpening)
  {
    if(passportPage == 1 && musicVolume != 0)
    {
      // TODO setStreamVolume(25 * musicVolume + 5);
    }
    return;
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::LarasHomePolaroid)
  {
    selectedOption = 0;
    return;
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::Pistols)
  {
    engine.getInventory().tryUse(engine.getObjectManager().getLara(), engine::TR1ItemId::Pistols);
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::Shotgun)
  {
    engine.getInventory().tryUse(engine.getObjectManager().getLara(), engine::TR1ItemId::Shotgun);
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::Magnums)
  {
    engine.getInventory().tryUse(engine.getObjectManager().getLara(), engine::TR1ItemId::Magnums);
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::Uzis)
  {
    engine.getInventory().tryUse(engine.getObjectManager().getLara(), engine::TR1ItemId::Uzis);
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::SmallMedipack)
  {
    engine.getInventory().tryUse(engine.getObjectManager().getLara(), engine::TR1ItemId::SmallMedipack);
  }
  else if(inventoryChosen.value() == engine::TR1ItemId::LargeMedipack)
  {
    engine.getInventory().tryUse(engine.getObjectManager().getLara(), engine::TR1ItemId::LargeMedipack);
  }

  if(mode != InventoryMode::TitleMode && musicVolume != 0)
  {
    // TODO setStreamVolume(25 * musicVolume + 5);
  }
}

MenuDisplay::MenuDisplay() = default;
MenuDisplay::~MenuDisplay() = default;
} // namespace menu
