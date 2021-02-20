#include "idleringmenustate.h"

#include "applyitemtransformmenustate.h"
#include "deflateringmenustate.h"
#include "donemenustate.h"
#include "engine/audioengine.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "rotateleftrightmenustate.h"
#include "switchringmenustate.h"
#include "util.h"

namespace menu
{
std::unique_ptr<MenuState> IdleRingMenuState::onFrame(ui::Ui& /*ui*/, engine::World& world, MenuDisplay& display)
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

  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu) && display.allowMenuClose)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
    display.inventoryChosen.reset();
    return create<DeflateRingMenuState>(create<DoneMenuState>(MenuResult::Closed));
  }

  if(m_autoSelect || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
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
      break;
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

void IdleRingMenuState::handleObject(ui::Ui& ui, engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(ui, world, object);
    idleRotation(world, object);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}
} // namespace menu
