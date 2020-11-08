#include "menustates.h"

#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "menudisplay.h"
#include "menuobject.h"
#include "menuring.h"

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

template<typename Unit, typename Type>
auto exactScale(const qs::quantity<Unit, Type>& value, const core::Frame& x, const core::Frame& max)
{
  const auto f = x.cast<float>() / max.cast<float>();
  return (value.template cast<float>() * f).template cast<Type>();
}
} // namespace

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
    return std::make_unique<PassportMenuState>(display.mode, m_ringTransform);

  bool animRunning = currentObject.selectedRotationY == currentObject.rotationY ? currentObject.animate() : false;
  if(animRunning)
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

void PassportMenuState::handleObject(engine::Engine& /*engine*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::unique_ptr<MenuState>
  PassportMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::Engine& engine, MenuDisplay& display)
{
  auto& passport = display.getCurrentRing().getSelectedObject();
  passport.type = engine::TR1ItemId::PassportOpening;

  bool animRunning = passport.selectedRotationY == passport.rotationY ? passport.animate() : false;
  if(animRunning)
    return nullptr;

  static constexpr int LoadGamePage = 0;
  static constexpr int SaveGamePage = 1;
  static constexpr int ExitGamePage = 2;

  static constexpr bool isInGame = true;      // FIXME
  static constexpr bool hasSavedGames = true; // FIXME

  display.objectTexts[0].reset();
  const auto localFrame = passport.goalFrame - passport.openFrame;
  auto page = localFrame / 5_frame;
  hid::AxisMovement forcePageTurn = hid::AxisMovement::Null;
  if(localFrame % 5_frame != 0_frame)
  {
    page = -1;
  }
  else if(m_forcePage.value_or(page) != page)
  {
    page = -1;
    if(page < *m_forcePage)
      forcePageTurn = hid::AxisMovement::Right;
    else
      forcePageTurn = hid::AxisMovement::Left;
  }

  switch(page)
  {
  case LoadGamePage:
    if(!hasSavedGames || display.mode == InventoryMode::SaveMode)
    {
      forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(display.passportText == nullptr)
    {
      display.passportText = std::make_unique<ui::Label>(0, -16, "Load Game");
      display.passportText->alignY = ui::Label::Alignment::Bottom;
      display.passportText->alignX = ui::Label::Alignment::Center;
    }
    if(engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true)
       || display.mode == InventoryMode::LoadMode)
    {
      display.objectTexts[0].reset();
      display.objectTexts[2].reset();
      BOOST_LOG_TRIVIAL(error) << "load game dialog not implemented yet";
      return nullptr;
    }
    break;
  case SaveGamePage:
    if(display.mode == InventoryMode::DeathMode)
    {
      // can't save when dead, so just skip this page
      if(passport.animDirection == -1_frame)
        forcePageTurn = hid::AxisMovement::Left;
      else
        forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(display.passportText == nullptr)
    {
      display.passportText = std::make_unique<ui::Label>(
        0, -16, display.mode != InventoryMode::TitleMode && isInGame ? "Save Game" : "New Game");
      display.passportText->alignY = ui::Label::Alignment::Bottom;
      display.passportText->alignX = ui::Label::Alignment::Center;
    }
    if(engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true)
       || display.mode == InventoryMode::SaveMode)
    {
      if(display.mode != InventoryMode::TitleMode && isInGame)
      {
        display.objectTexts[0].reset();
        display.objectTexts[2].reset();
        BOOST_LOG_TRIVIAL(error) << "save game dialog not implemented yet";
        return nullptr;
      }
    }
    break;
  case ExitGamePage:
    if(display.passportText == nullptr)
    {
      display.passportText = std::make_unique<ui::Label>(0, -16, !isInGame ? "Exit Game" : "Exit to Title");
      display.passportText->alignY = ui::Label::Alignment::Bottom;
      display.passportText->alignX = ui::Label::Alignment::Center;
    }
    break;
  default: Expects(page == -1); break;
  }

  if(forcePageTurn == hid::AxisMovement::Left
     || engine.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    if(hasSavedGames)
    {
      passport.goalFrame -= 5_frame;
      passport.animDirection = -1_frame;
      if(passport.goalFrame < passport.openFrame)
      {
        passport.goalFrame = passport.openFrame;
      }
      else
      {
        engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
        display.passportText.reset();
      }
      return nullptr;
    }
    else if(display.mode != InventoryMode::DeathMode)
    {
      passport.goalFrame -= 5_frame;
      passport.animDirection = -1_frame;
      if(passport.goalFrame >= passport.openFrame + 5_frame)
      {
        display.passportText.reset();
      }
      else
      {
        passport.goalFrame = passport.openFrame + 5_frame;
      }
      return nullptr;
    }
  }
  else if(forcePageTurn == hid::AxisMovement::Right
          || engine.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    passport.goalFrame += 5_frame;
    passport.animDirection = 1_frame;
    if(passport.goalFrame > passport.lastMeshAnimFrame - 6_frame)
    {
      passport.goalFrame = passport.lastMeshAnimFrame - 6_frame;
    }
    else
    {
      engine.getPresenter().getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
      display.passportText.reset();
    }
    return nullptr;
  }
  else if(engine.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    if(!m_allowExit)
    {
      return nullptr;
    }

    if(page == ExitGamePage)
    {
      passport.animDirection = 1_frame;
      passport.goalFrame = passport.lastMeshAnimFrame - 1_frame;
    }
    else
    {
      passport.goalFrame = 0_frame;
      passport.animDirection = -1_frame;
    }
    display.passportText.reset();
  }
  else if(engine.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    display.passportText.reset();
    if(page == ExitGamePage)
    {
      passport.animDirection = 1_frame;
      passport.goalFrame = passport.lastMeshAnimFrame - 1_frame;
    }
    else
    {
      passport.goalFrame = 0_frame;
      passport.animDirection = -1_frame;
    }
  }

  return nullptr;
}

PassportMenuState::PassportMenuState(InventoryMode mode, const std::shared_ptr<MenuRingTransform>& ringTransform)
    : MenuState{ringTransform}
    , m_allowExit{mode != InventoryMode::DeathMode}
    , m_allowSave{mode != InventoryMode::DeathMode}
    , m_forcePage{mode == InventoryMode::LoadMode   ? std::optional<int>{0}
                  : mode == InventoryMode::SaveMode ? std::optional<int>{1}
                                                    : std::nullopt}
{
}
} // namespace menu
