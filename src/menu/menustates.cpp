#include "menustates.h"

#include "engine/audioengine.h"
#include "engine/engine.h"
#include "engine/presenter.h"
#include "engine/world.h"
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

void idleRotation(engine::World& world, MenuObject& object)
{
  if(world.getPresenter().getInputHandler().getInputState().xMovement == hid::AxisMovement::Null)
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

std::vector<std::filesystem::path> getSavegames(const engine::World& world)
{
  if(!std::filesystem::is_directory(world.getEngine().getSavegamePath()))
    return {};

  std::vector<std::filesystem::path> result;
  for(const auto& p : std::filesystem::directory_iterator(world.getEngine().getSavegamePath()))
  {
    if(!p.is_regular_file() || p.path().extension() != ".meta")
      continue;

    result.emplace_back(p.path());
  }

  return result;
}

std::string makeSavegameBasename(size_t n)
{
  return "save_" + std::to_string(n);
}
} // namespace

void ResetItemTransformMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
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
                                                                engine::World& /*world*/,
                                                                MenuDisplay& /*display*/)
{
  if(m_duration != 0_frame)
  {
    m_duration -= 1_frame;
    return nullptr;
  }

  return std::move(m_next);
}

std::unique_ptr<MenuState>
  FinishItemAnimationMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& world, MenuDisplay& display)
{
  display.updateRingTitle();

  auto& object = display.getCurrentRing().getSelectedObject();
  if(object.animate())
    return nullptr; // play full animation until its end

  if(object.type == engine::TR1ItemId::PassportOpening)
  {
    object.type = engine::TR1ItemId::PassportClosed;
    object.meshAnimFrame = 0_frame;
    object.initModel(world);
  }

  return std::move(m_next);
}

void FinishItemAnimationMenuState::handleObject(engine::World& /*world*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::unique_ptr<MenuState>
  DeselectingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
{
  display.updateRingTitle();
  return create<IdleRingMenuState>(false);
}

void DeselectingMenuState::handleObject(engine::World& /*world*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

DeselectingMenuState::DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                           engine::World& world)
    : MenuState{ringTransform}
{
  world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
}

std::unique_ptr<MenuState>
  IdleRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& world, MenuDisplay& display)
{
  display.updateRingTitle();

  if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right)
     && display.getCurrentRing().list.size() > 1)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuMove, nullptr);
    return create<RotateLeftRightMenuState>(true, display.getCurrentRing(), std::move(display.m_currentState));
  }

  if(world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left)
     && display.getCurrentRing().list.size() > 1)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuMove, nullptr);
    return create<RotateLeftRightMenuState>(false, display.getCurrentRing(), std::move(display.m_currentState));
  }

  if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true) && display.allowMenuClose)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
    display.inventoryChosen.reset();
    return create<DeflateRingMenuState>(create<DoneMenuState>(MenuResult::Closed));
  }

  if(m_autoSelect || world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    display.passOpen = true;

    auto& currentObject = display.getCurrentRing().getSelectedObject();

    switch(currentObject.type)
    {
    case engine::TR1ItemId::Compass:
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionSelect2, nullptr);
      break;
    case engine::TR1ItemId::LarasHomePolaroid:
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuHome, nullptr);
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeflateRingMenuState>(create<DoneMenuState>(MenuResult::LaraHome))));
    case engine::TR1ItemId::DirectionKeys:
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::LowTone, nullptr);
      break;
    case engine::TR1ItemId::Pistols:
    case engine::TR1ItemId::Shotgun:
    case engine::TR1ItemId::Magnums:
    case engine::TR1ItemId::Uzis:
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionSelect1, nullptr);
      break;
    default: world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr); break;
    }

    currentObject.goalFrame = currentObject.openFrame;
    currentObject.animDirection = 1_frame;
    return create<ApplyItemTransformMenuState>();
  }

  if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward)
     && display.currentRingIndex > 0)
  {
    return create<DeflateRingMenuState>(create<SwitchRingMenuState>(display.currentRingIndex - 1, false));
  }
  else if(world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Backward)
          && display.currentRingIndex + 1 < display.rings.size())
  {
    return create<DeflateRingMenuState>(create<SwitchRingMenuState>(display.currentRingIndex + 1, false));
  }

  return nullptr;
}

void IdleRingMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
    idleRotation(world, object);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}

std::unique_ptr<MenuState>
  SwitchRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
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

  return create<InflateRingMenuState>();
}

void SwitchRingMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
    idleRotation(world, object);
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
  SelectedMenuState::onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display)
{
  auto& currentObject = display.getCurrentRing().getSelectedObject();
  if(currentObject.type == engine::TR1ItemId::PassportClosed)
    return create<PassportMenuState>(display.mode);

  if(currentObject.selectedRotationY == currentObject.rotationY && currentObject.animate())
    return nullptr;

  const bool autoSelect = display.doOptions(img, world, currentObject);
  if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    if(display.rings.size() > 1)
    {
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeselectingMenuState>(world)));
    }
    else
    {
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeflateRingMenuState>(create<DoneMenuState>(MenuResult::Closed))));
    }
  }
  else if(autoSelect || world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    display.inventoryChosen = currentObject.type;
    if(display.mode == InventoryMode::TitleMode
       && (currentObject.type == engine::TR1ItemId::Sunglasses
           || currentObject.type == engine::TR1ItemId::CassettePlayer
           || currentObject.type == engine::TR1ItemId::DirectionKeys
           || currentObject.type == engine::TR1ItemId::Flashlight))
    {
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeselectingMenuState>(world)));
    }
    else
    {
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeflateRingMenuState>(create<DoneMenuState>(MenuResult::Closed))));
    }
  }

  return nullptr;
}

void SelectedMenuState::handleObject(engine::World& /*world*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::unique_ptr<MenuState>
  InflateRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
{
  if(m_duration == 0_frame)
  {
    bool doAutoSelect = false;
    if(display.rings.size() == 1 && !display.passOpen)
    {
      doAutoSelect = true;
    }

    return create<IdleRingMenuState>(doAutoSelect);
  }

  m_duration -= 1_frame;
  m_ringTransform->ringRotation
    = display.getCurrentRing().getCurrentObjectAngle() - exactScale(90_deg, m_duration, Duration);
  m_ringTransform->cameraRotX = exactScale(m_initialCameraRotX, m_duration, Duration);
  m_ringTransform->radius += m_radiusSpeed;
  m_ringTransform->cameraPos.Y += m_cameraSpeedY;
  return nullptr;
}

void InflateRingMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  display.clearMenuObjectDescription();
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    idleRotation(world, object);
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

void ApplyItemTransformMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
  {
    zeroRotation(object, 256_au);
    return;
  }

  display.updateMenuObjectDescription(world, object);
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
  ApplyItemTransformMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
{
  display.updateRingTitle();

  if(m_duration != Duration)
  {
    m_duration += 1_frame;
    return nullptr;
  }

  return create<SelectedMenuState>();
}

void DeflateRingMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(world, object);
  }
}

std::unique_ptr<MenuState>
  DeflateRingMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& /*display*/)
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

void DoneMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(world, object);
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(world, object);
  }
}

std::unique_ptr<MenuState>
  DoneMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& world, MenuDisplay& display)
{
  if(!display.allowMenuClose)
  {
    return nullptr;
  }

  display.result = m_result;

  switch(display.inventoryChosen.value_or(engine::TR1ItemId::Lara))
  {
  case engine::TR1ItemId::Pistols:
    world.getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Pistols);
    break;
  case engine::TR1ItemId::Shotgun:
    world.getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Shotgun);
    break;
  case engine::TR1ItemId::Magnums:
    world.getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Magnums);
    break;
  case engine::TR1ItemId::Uzis:
    world.getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Uzis);
    break;
  case engine::TR1ItemId::SmallMedipack:
    world.getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::SmallMedipack);
    break;
  case engine::TR1ItemId::LargeMedipack:
    world.getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::LargeMedipack);
    break;
  default: break;
  }

  return nullptr;
}

void RotateLeftRightMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
    display.updateMenuObjectDescription(world, object);
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
  RotateLeftRightMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& display)
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

void PassportMenuState::handleObject(engine::World& /*world*/, MenuDisplay& display, MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

std::unique_ptr<MenuState>
  PassportMenuState::onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& display)
{
  auto& passport = display.getCurrentRing().getSelectedObject();
  passport.type = engine::TR1ItemId::PassportOpening;
  passport.initModel(world);

  if(passport.selectedRotationY == passport.rotationY && passport.animate())
    return nullptr;

  const bool isInGame = display.mode != InventoryMode::TitleMode && display.mode != InventoryMode::DeathMode;
  const bool hasSavedGames = !getSavegames(world).empty();

  display.objectTexts[0].reset();
  const auto localFrame = passport.goalFrame - passport.openFrame;
  auto page = localFrame / FramesPerPage;
  hid::AxisMovement forcePageTurn = hid::AxisMovement::Null;
  if(localFrame % FramesPerPage != 0_frame)
  {
    page = -1;
  }
  else if(m_forcePage.value_or(page) != page)
  {
    page = -1;
    if(page < *m_forcePage)
      forcePageTurn = hid::AxisMovement::Right;
    else if(page > *m_forcePage)
      forcePageTurn = hid::AxisMovement::Left;
    else
      m_forcePage.reset();
  }

  switch(page)
  {
  case LoadGamePage:
    if(!hasSavedGames || display.mode == InventoryMode::SaveMode)
    {
      forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(m_passportText == nullptr)
    {
      m_passportText = std::make_unique<ui::Label>(glm::ivec2{0, -16}, "Load Game");
      m_passportText->alignX = ui::Label::Alignment::Center;
      m_passportText->alignY = ui::Label::Alignment::Bottom;
    }
    if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true)
       || display.mode == InventoryMode::LoadMode)
    {
      display.objectTexts[0].reset();
      display.objectTexts[2].reset();
      BOOST_LOG_TRIVIAL(error) << "load game dialog not implemented yet";
      return nullptr;
    }
    break;
  case SaveGamePage:
    if(!m_allowSave && display.mode != InventoryMode::TitleMode)
    {
      // can't save when dead, so just skip this page
      if(passport.animDirection == -1_frame)
        forcePageTurn = hid::AxisMovement::Left;
      else
        forcePageTurn = hid::AxisMovement::Right;
      break;
    }
    if(m_passportText == nullptr)
    {
      m_passportText
        = std::make_unique<ui::Label>(glm::ivec2{0, -16}, m_allowSave && isInGame ? "Save Game" : "New Game");
      m_passportText->alignX = ui::Label::Alignment::Center;
      m_passportText->alignY = ui::Label::Alignment::Bottom;
    }
    if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
    {
      if(display.mode == InventoryMode::SaveMode || (m_allowSave && isInGame))
      {
        display.objectTexts[0].reset();
        display.objectTexts[2].reset();
        return create<SavegameListMenuState>(std::move(display.m_currentState), m_passportText->text);
      }
      else
      {
        return create<DoneMenuState>(MenuResult::NewGame);
      }
    }
    break;
  case ExitGamePage:
    if(m_passportText == nullptr)
    {
      m_passportText = std::make_unique<ui::Label>(glm::ivec2{0, -16}, !isInGame ? "Exit Game" : "Exit to Title");
      m_passportText->alignX = ui::Label::Alignment::Center;
      m_passportText->alignY = ui::Label::Alignment::Bottom;
    }
    if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
    {
      display.result = !isInGame ? MenuResult::ExitGame : MenuResult::ExitToTitle;
    }
    break;
  default: Expects(page == -1); break;
  }

  if(m_passportText != nullptr)
    m_passportText->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  if(forcePageTurn == hid::AxisMovement::Left
     || world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Left))
  {
    if(hasSavedGames)
    {
      passport.goalFrame -= FramesPerPage;
      passport.animDirection = -1_frame;
      if(passport.goalFrame < passport.openFrame)
      {
        passport.goalFrame = passport.openFrame;
      }
      else
      {
        world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
        m_passportText.reset();
      }
      return nullptr;
    }
    else if(m_allowSave || display.mode == InventoryMode::TitleMode)
    {
      passport.goalFrame -= FramesPerPage;
      passport.animDirection = -1_frame;
      if(const auto firstFrame = passport.openFrame + FramesPerPage; passport.goalFrame < firstFrame)
      {
        passport.goalFrame = firstFrame;
      }
      else
      {
        world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
        m_passportText.reset();
      }
      return nullptr;
    }
  }
  else if(forcePageTurn == hid::AxisMovement::Right
          || world.getPresenter().getInputHandler().getInputState().xMovement.justChangedTo(hid::AxisMovement::Right))
  {
    passport.goalFrame += FramesPerPage;
    passport.animDirection = 1_frame;
    if(const auto lastFrame = passport.lastMeshAnimFrame - FramesPerPage - 1_frame; passport.goalFrame > lastFrame)
    {
      passport.goalFrame = lastFrame;
    }
    else
    {
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuGamePageTurn, nullptr);
      m_passportText.reset();
    }
    return nullptr;
  }
  else if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    if(!m_allowExit && display.mode != InventoryMode::TitleMode)
      return nullptr;

    return close(display, page, passport);
  }
  else if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    return close(display, page, passport);
  }

  return nullptr;
}

PassportMenuState::PassportMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform, InventoryMode mode)
    : MenuState{ringTransform}
    , m_allowExit{mode != InventoryMode::DeathMode && mode != InventoryMode::TitleMode}
    , m_allowSave{mode != InventoryMode::DeathMode && mode != InventoryMode::TitleMode}
    , m_forcePage{mode == InventoryMode::LoadMode   ? std::optional<int>{0}
                  : mode == InventoryMode::SaveMode ? std::optional<int>{1}
                                                    : std::nullopt}
{
}

std::unique_ptr<MenuState> PassportMenuState::close(MenuDisplay& /*display*/, int page, MenuObject& passport)
{
  m_passportText.reset();

  if(page == ExitGamePage)
  {
    passport.goalFrame = passport.lastMeshAnimFrame - 1_frame;
    passport.animDirection = 1_frame;
  }
  else
  {
    passport.goalFrame = 0_frame;
    passport.animDirection = -1_frame;
  }

  return create<FinishItemAnimationMenuState>(create<SetItemTypeMenuState>(
    engine::TR1ItemId::PassportClosed, create<ResetItemTransformMenuState>(create<IdleRingMenuState>(false))));
}

std::unique_ptr<MenuState>
  SetItemTypeMenuState::onFrame(gl::Image<gl::SRGBA8>& /*img*/, engine::World& /*world*/, MenuDisplay& /*display*/)
{
  return std::move(m_next);
}

void SetItemTypeMenuState::handleObject(engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    object.type = m_type;
    object.initModel(world);
  }
}

SavegameListMenuState::SavegameListMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform,
                                             std::unique_ptr<MenuState> previous,
                                             const std::string& heading)
    : MenuState{ringTransform}
    , m_previous{std::move(previous)}
    , m_heading{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - LineHeight - 10}, heading)}
    , m_background{std::make_unique<ui::Label>(glm::ivec2{0, YOffset - LineHeight - 12}, " ")}
{
  m_heading->alignX = ui::Label::Alignment::Center;
  m_heading->alignY = ui::Label::Alignment::Bottom;
  m_heading->addBackground({PixelWidth - 4, 0}, {0, 0});
  m_heading->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGBA8{16, 128, 56, 128}, gl::SRGBA8{0, 0, 0, 0});
  m_heading->outline = true;

  m_background->alignX = ui::Label::Alignment::Center;
  m_background->alignY = ui::Label::Alignment::Bottom;
  m_background->addBackground({PixelWidth, LineHeight + TotalHeight + 12}, {0, 0});

  m_background->backgroundGouraud = ui::Label::makeBackgroundCircle(gl::SRGBA8{0, 96, 0, 96}, gl::SRGBA8{0, 32, 0, 32});
  m_background->outline = true;

  for(size_t i = 0; i < TotalSlots; ++i)
  {
    const auto line = i % PerPage;
    auto lbl = std::make_unique<ui::Label>(glm::ivec2{0, YOffset + line * LineHeight},
                                           "- EMPTY SLOT " + std::to_string(i + 1));
    lbl->alignX = ui::Label::Alignment::Center;
    lbl->alignY = ui::Label::Alignment::Bottom;
    m_labels.emplace_back(std::move(lbl));
  }
}

void SavegameListMenuState::handleObject(engine::World& /*world*/, MenuDisplay& /*display*/, MenuObject& /*object*/)
{
}

std::unique_ptr<MenuState>
  SavegameListMenuState::onFrame(gl::Image<gl::SRGBA8>& img, engine::World& world, MenuDisplay& /*display*/)
{
  m_background->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  const auto page = m_selected / PerPage;
  const auto first = page * PerPage;
  const auto last = std::min(first + PerPage, m_labels.size());
  Expects(first < last);
  for(size_t i = first; i < last; ++i)
  {
    const auto& lbl = m_labels.at(i);
    if(m_selected == i)
    {
      lbl->addBackground(glm::ivec2{PixelWidth - 12, 16}, glm::ivec2{0});
      lbl->outline = true;
    }
    else
    {
      lbl->removeBackground();
      lbl->outline = false;
    }
    lbl->draw(world.getPresenter().getTrFont(), img, world.getPalette());
  }

  if(!m_heading->text.empty())
    m_heading->draw(world.getPresenter().getTrFont(), img, world.getPalette());

  if(m_selected > 0
     && world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward))
  {
    --m_selected;
  }
  else if(m_selected < TotalSlots - 1
          && world.getPresenter().getInputHandler().getInputState().zMovement.justChangedTo(
            hid::AxisMovement::Backward))
  {
    ++m_selected;
  }
  else if(world.getPresenter().getInputHandler().getInputState().action.justChangedTo(true))
  {
    world.getPresenter().drawLoadingScreen("Saving...");
    BOOST_LOG_TRIVIAL(info) << "Save";
    serialization::Serializer<engine::World>::save(
      world.getEngine().getSavegamePath() / (makeSavegameBasename(m_selected) + ".yaml"), world, world);
    return std::move(m_previous);
  }
  else if(world.getPresenter().getInputHandler().getInputState().menu.justChangedTo(true))
  {
    return std::move(m_previous);
  }

  return nullptr;
}
} // namespace menu
