#include "menudisplay.h"

#include "core/pybindmodule.h"
#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "hid/inputhandler.h"
#include "menuring.h"
#include "ui/util.h"

namespace menu
{
namespace
{
void rotateForSelection(MenuObject& object)
{
  if(object.rotationY == object.selectedRotationY)
    return;

  if(const auto dy = object.selectedRotationY - object.rotationY; dy > 0_deg && dy < 180_deg)
  {
    object.rotationY += 1024_au;
  }
  else
  {
    object.rotationY -= 1024_au;
  }
  object.rotationY -= object.rotationY % 1024_au;
}

void idleRotation(engine::Engine& engine, MenuObject& object)
{
  if(engine.getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Null)
  {
    object.rotationY += 256_au;
  }
}

void zeroRotation(MenuObject& object, const core::Angle& speed)
{
  BOOST_ASSERT(speed >= 0_deg);

  if(object.rotationY < 0_deg)
  {
    object.rotationY += speed;
    if(object.rotationY > 0_deg)
      object.rotationY = 0_deg;
  }
  else if(object.rotationY > 0_deg)
  {
    object.rotationY -= speed;
    if(object.rotationY < 0_deg)
      object.rotationY = 0_deg;
  }
}

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

template<typename Unit, typename Type>
auto exactScale(const qs::quantity<Unit, Type>& value, const core::Frame& x, const core::Frame& max)
{
  const auto f = x.cast<float>() / max.cast<float>();
  return (value.template cast<float>() * f).template cast<Type>();
}
} // namespace

void MenuDisplay::updateRingTitle()
{
  if(rings.size() == 1)
    return;

  if(objectTexts[2] == nullptr)
  {
    objectTexts[2] = std::make_shared<ui::Label>(0, 26, getCurrentRing().title);
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
      objectTexts[3] = std::make_shared<ui::Label>(20, 28, "[");
      objectTexts[4] = std::make_shared<ui::Label>(-20, 28, "[");
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
      objectTexts[5] = std::make_shared<ui::Label>(20, -15, "]");
      objectTexts[5]->alignY = ui::Label::Alignment::Bottom;
      objectTexts[6] = std::make_shared<ui::Label>(-20, -15, "]");
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
          objectTexts[0] = std::make_shared<ui::Label>(0, -16, objectName.value());
        }
      }
    }

    if(objectTexts[0] == nullptr)
      objectTexts[0] = std::make_shared<ui::Label>(0, -16, object.name);

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
    objectTexts[1] = std::make_shared<ui::Label>(64, -56, makeAmmoString(std::to_string(totalItemCount) + suffix));
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
  if(ammoText != nullptr)
    ammoText->draw(engine.getPresenter().getTrFont(), img, engine.getPalette());
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
    rings.emplace_back(std::make_shared<MenuRing>(MenuRing::Type::Inventory, "INVENTORY", getMainRingObjects(engine)));

  if(mode != InventoryMode::KeysMode)
  {
    rings.emplace_back(std::make_shared<MenuRing>(MenuRing::Type::Options,
                                                  mode == InventoryMode::DeathMode ? "GAME OVER" : "OPTION",
                                                  getOptionRingObjects(mode == InventoryMode::TitleMode)));
  }

  if(mode == InventoryMode::KeysMode || mode == InventoryMode::GameMode)
  {
    rings.emplace_back(std::make_shared<MenuRing>(MenuRing::Type::Items, "ITEMS", getKeysRingObjects(engine)));
    if(rings.back()->list.empty())
      rings.pop_back();

    if(mode == InventoryMode::KeysMode && rings.empty())
      return false;
  }

  Ensures(!rings.empty());

  ammoText.reset();
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

void ResetItemTransformMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(engine, object);
    object.baseRotationX = exactScale(object.selectedBaseRotationX, m_duration, Duration);
    object.rotationX = exactScale(object.selectedRotationX, m_duration, Duration);
    object.positionZ = exactScale(object.selectedPositionZ, m_duration, Duration);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

std::unique_ptr<MenuState> ResetItemTransformMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/,
                                                                engine::Engine& /*engine*/,
                                                                MenuDisplay& /*display*/)
{
  if(m_duration != 0_frame)
  {
    m_duration -= 1_frame;
    return nullptr;
  }

  return std::move(m_next);
}

std::unique_ptr<MenuState> FinishItemAnimationMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/,
                                                                 engine::Engine& /*engine*/,
                                                                 MenuDisplay& display)
{
  display.updateRingTitle();

  auto& object = display.getCurrentRing().getSelectedObject();
  if(object.animate())
    return nullptr; // play full animation until its end

  if(object.type == engine::TR1ItemId::PassportOpening)
  {
    object.type = engine::TR1ItemId::PassportClosed;
    object.meshAnimFrame = 0_frame;
  }

  return std::move(m_next);
}

void FinishItemAnimationMenuState::handleObject(engine::Engine& /*engine*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::unique_ptr<MenuState>
  DeselectingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& display)
{
  display.updateRingTitle();
  return std::make_unique<IdleRingMenuState>(m_ringTransform, false);
}

void DeselectingMenuState::handleObject(engine::Engine& /*engine*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

DeselectingMenuState::DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           engine::Engine& engine)
    : MenuState{ringTransform}
{
  engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
}

std::unique_ptr<MenuState>
  IdleRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& engine, MenuDisplay& display)
{
  display.updateRingTitle();

  if(engine.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right)
     && display.getCurrentRing().list.size() > 1)
  {
    engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuMove, nullptr);
    return std::make_unique<RotateLeftRightMenuState>(
      m_ringTransform, true, display.getCurrentRing(), std::move(display.m_currentState));
  }

  if(engine.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left)
     && display.getCurrentRing().list.size() > 1)
  {
    engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuMove, nullptr);
    return std::make_unique<RotateLeftRightMenuState>(
      m_ringTransform, false, display.getCurrentRing(), std::move(display.m_currentState));
  }

  if(engine.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true) && display.allowMenuClose)
  {
    engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
    display.inventoryChosen.reset();
    return std::make_unique<DeflateRingMenuState>(m_ringTransform, std::make_unique<DoneMenuState>(m_ringTransform));
  }

  if(m_autoSelect || engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    display.passOpen = true;

    auto& currentObject = display.getCurrentRing().getSelectedObject();

    switch(currentObject.type)
    {
    case engine::TR1ItemId::Compass:
      engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionSelect2, nullptr);
      break;
    case engine::TR1ItemId::LarasHomePolaroid:
      engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuHome, nullptr);
      break;
    case engine::TR1ItemId::DirectionKeys:
      engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::LowTone, nullptr);
      break;
    case engine::TR1ItemId::Pistols:
    case engine::TR1ItemId::Shotgun:
    case engine::TR1ItemId::Magnums:
    case engine::TR1ItemId::Uzis:
      engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionSelect1, nullptr);
      break;
    default:
      engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr);
      break;
    }

    currentObject.goalFrame = currentObject.openFrame;
    currentObject.animDirection = 1_frame;
    return std::make_unique<ApplyItemTransformMenuState>(m_ringTransform);
  }

  if(engine.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward)
     && display.currentRingIndex > 0)
  {
    return std::make_unique<DeflateRingMenuState>(
      m_ringTransform, std::make_unique<SwitchRingMenuState>(m_ringTransform, display.currentRingIndex - 1, false));
  }
  else if(engine.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Backward)
          && display.currentRingIndex + 1 < display.rings.size())
  {
    return std::make_unique<DeflateRingMenuState>(
      m_ringTransform, std::make_unique<SwitchRingMenuState>(m_ringTransform, display.currentRingIndex + 1, false));
  }

  return nullptr;
}

void IdleRingMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(engine, object);
    idleRotation(engine, object);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

std::unique_ptr<MenuState>
  SwitchRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& display)
{
  if(m_duration != Duration)
  {
    m_duration += 1_frame;
    m_ringTransform->radius -= m_radiusSpeed;
    m_ringTransform->ringRotation -= 180_deg / Duration * 1_frame;
    m_ringTransform->cameraRotX = exactScale(m_targetCameraRotX, m_duration, Duration);
    return nullptr;
  }

  display.currentRingIndex = m_next;
  m_ringTransform->cameraRotX = m_targetCameraRotX;

  return std::make_unique<InflateRingMenuState>(m_ringTransform);
}

void SwitchRingMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(engine, object);
    idleRotation(engine, object);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

SwitchRingMenuState::SwitchRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                         size_t next,
                                         bool down)
    : MenuState{ringTransform}
    , m_next{next}
    , m_down{down}
{
}

std::unique_ptr<MenuState>
  SelectedMenuState::onFrame(gl::Image<gl::SRGBA8>& img, engine::Engine& engine, MenuDisplay& display)
{
  auto& currentObject = display.getCurrentRing().getSelectedObject();
  if(currentObject.type == engine::TR1ItemId::PassportClosed)
  {
    currentObject.type = engine::TR1ItemId::PassportOpening;
  }
  bool animGoalReached = currentObject.selectedRotationY == currentObject.rotationY ? currentObject.animate() : false;
  if(animGoalReached)
    return nullptr;

  const bool autoSelect = display.doOptions(img, engine, currentObject);
  if(engine.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    if(display.rings.size() > 1)
    {
      return std::make_unique<FinishItemAnimationMenuState>(
        m_ringTransform,
        std::make_unique<ResetItemTransformMenuState>(m_ringTransform,
                                                      std::make_unique<DeselectingMenuState>(m_ringTransform, engine)));
    }
    else
    {
      return std::make_unique<FinishItemAnimationMenuState>(
        m_ringTransform,
        std::make_unique<ResetItemTransformMenuState>(
          m_ringTransform,
          std::make_unique<DeflateRingMenuState>(m_ringTransform, std::make_unique<DoneMenuState>(m_ringTransform))));
    }
  }
  else if(autoSelect || engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    display.inventoryChosen = currentObject.type;
    if(display.mode == InventoryMode::TitleMode
       && (currentObject.type == engine::TR1ItemId::Sunglasses
           || currentObject.type == engine::TR1ItemId::CassettePlayer
           || currentObject.type == engine::TR1ItemId::DirectionKeys
           || currentObject.type == engine::TR1ItemId::Flashlight))
    {
      return std::make_unique<FinishItemAnimationMenuState>(
        m_ringTransform,
        std::make_unique<ResetItemTransformMenuState>(m_ringTransform,
                                                      std::make_unique<DeselectingMenuState>(m_ringTransform, engine)));
    }
    else
    {
      return std::make_unique<FinishItemAnimationMenuState>(
        m_ringTransform,
        std::make_unique<ResetItemTransformMenuState>(
          m_ringTransform,
          std::make_unique<DeflateRingMenuState>(m_ringTransform, std::make_unique<DoneMenuState>(m_ringTransform))));
    }
  }

  return nullptr;
}

void SelectedMenuState::handleObject(engine::Engine& /*engine*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::unique_ptr<MenuState>
  InflateRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& display)
{
  if(m_duration == 0_frame)
  {
    bool doAutoSelect = false;
    if(display.rings.size() == 1 && !display.passOpen)
    {
      doAutoSelect = true;
    }

    return std::make_unique<IdleRingMenuState>(m_ringTransform, doAutoSelect);
  }

  m_duration -= 1_frame;
  m_ringTransform->ringRotation
    = display.getCurrentRing().getCurrentObjectAngle() - exactScale(90_deg, m_duration, Duration);
  m_ringTransform->cameraRotX = exactScale(m_initialCameraRotX, m_duration, Duration);
  m_ringTransform->radius += m_radiusSpeed;
  m_ringTransform->cameraPos.Y += m_cameraSpeedY;
  return nullptr;
}

void InflateRingMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  display.clearMenuObjectDescription();
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    idleRotation(engine, object);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

InflateRingMenuState::InflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
    : MenuState{ringTransform}
{
}

void ApplyItemTransformMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
  {
    zeroRotation(object, 256_au);
    return;
  }

  display.updateMenuObjectDescription(engine, object);
  object.baseRotationX = exactScale(object.selectedBaseRotationX, m_duration, Duration);
  object.rotationX = exactScale(object.selectedRotationX, m_duration, Duration);
  object.positionZ = exactScale(object.selectedPositionZ, m_duration, Duration);

  if(object.rotationY != object.selectedRotationY)
  {
    if(const auto dy = object.selectedRotationY - object.rotationY; dy > 0_deg && dy < 180_deg)
    {
      object.rotationY += 1024_au;
    }
    else
    {
      object.rotationY -= 1024_au;
    }
    object.rotationY -= object.rotationY % 1024_au;
  }
}

std::unique_ptr<MenuState>
  ApplyItemTransformMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& display)
{
  display.updateRingTitle();

  if(m_duration != Duration)
  {
    m_duration += 1_frame;
    return nullptr;
  }

  return std::make_unique<SelectedMenuState>(m_ringTransform);
}

void DeflateRingMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(engine, object);
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(engine, object);
  }
}

std::unique_ptr<MenuState>
  DeflateRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& /*display*/)
{
  if(m_duration == 0_frame)
    return std::move(m_next);

  m_duration -= 1_frame;
  m_ringTransform->ringRotation -= 180_deg / Duration * 1_frame;
  m_ringTransform->radius = exactScale(m_initialRadius, m_duration, Duration);
  m_ringTransform->cameraPos.Y += m_cameraSpeedY;
  return nullptr;
}

DeflateRingMenuState::DeflateRingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           std::unique_ptr<MenuState> next)
    : MenuState{ringTransform}
    , m_next{std::move(next)}
{
}

void DoneMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(engine, object);
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(engine, object);
  }
}

std::unique_ptr<MenuState>
  DoneMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& display)
{
  display.isDone = true;
  return nullptr;
}

void RotateLeftRightMenuState::handleObject(engine::Engine& engine, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
    display.updateMenuObjectDescription(engine, object);
  zeroRotation(object, 512_au);
}

RotateLeftRightMenuState::RotateLeftRightMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                                   bool left,
                                                   const MenuRing& ring,
                                                   std::unique_ptr<MenuState>&& prev)
    : MenuState{ringTransform}
    , m_rotSpeed{(left ? -ring.getAnglePerItem() : ring.getAnglePerItem()) / Duration}
    , m_prev{std::move(prev)}
{
  m_targetObject = ring.currentObject + (left ? -1 : 1);
  if(m_targetObject == std::numeric_limits<size_t>::max())
  {
    m_targetObject = ring.list.size() - 1;
  }
  if(m_targetObject >= ring.list.size())
  {
    m_targetObject = 0;
  }
}

std::unique_ptr<MenuState>
  RotateLeftRightMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& /*engine*/, MenuDisplay& display)
{
  display.clearMenuObjectDescription();
  m_ringTransform->ringRotation += m_rotSpeed * 1_frame;
  m_duration -= 1_frame;
  if(m_duration != 0_frame)
    return nullptr;

  display.getCurrentRing().currentObject = m_targetObject;
  m_ringTransform->ringRotation = display.getCurrentRing().getCurrentObjectAngle();
  return std::move(m_prev);
}
} // namespace menu
