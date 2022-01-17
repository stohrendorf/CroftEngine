#include "idleringmenustate.h"

#include "applyitemtransformmenustate.h"
#include "core/units.h"
#include "deflateringmenustate.h"
#include "donemenustate.h"
#include "engine/audioengine.h"
#include "engine/items_tr1.h"
#include "engine/presenter.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "hid/inputstate.h"
#include "menudisplay.h"
#include "menuobject.h"
#include "menuring.h"
#include "menustate.h"
#include "rotateleftrightmenustate.h"
#include "switchringmenustate.h"
#include "ui/core.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util.h"

#include <glm/vec2.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace menu
{
std::unique_ptr<MenuState> IdleRingMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  auto& presenter = world.getPresenter();

  {
    const auto& currentObject = display.getCurrentRing().getSelectedObject();
    ui::Text text{world.getItemTitle(currentObject.type).value_or(currentObject.name)};
    text.draw(ui,
              presenter.getTrFont(),
              {(ui.getSize().x - text.getWidth()) / 2, ui.getSize().y - RingInfoYMargin - ui::FontHeight});

    MenuDisplay::drawMenuObjectDescription(ui, world, currentObject);
  }

  if(presenter.getInputHandler().getInputState().xMovement == hid::AxisMovement::Right
     && display.getCurrentRing().list.size() > 1)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuMove, nullptr);
    return create<RotateLeftRightMenuState>(true, display.getCurrentRing(), std::move(display.m_currentState));
  }

  if(presenter.getInputHandler().getInputState().xMovement == hid::AxisMovement::Left
     && display.getCurrentRing().list.size() > 1)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuMove, nullptr);
    return create<RotateLeftRightMenuState>(false, display.getCurrentRing(), std::move(display.m_currentState));
  }

  if(presenter.getInputHandler().hasDebouncedAction(hid::Action::Menu) && display.allowMenuClose)
  {
    world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionEscape, nullptr);
    display.inventoryChosen.reset();
    return create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack,
                                        create<DoneMenuState>(MenuResult::Closed));
  }

  if(m_autoSelect || presenter.getInputHandler().hasDebouncedAction(hid::Action::Action))
  {
    display.passOpen = true;

    auto& currentObject = display.getCurrentRing().getSelectedObject();

    switch(currentObject.type)
    {
    case engine::TR1ItemId::Compass:
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionSelect1, nullptr);
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
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionSelect2, nullptr);
      break;
    default:
      world.getAudioEngine().playSoundEffect(engine::TR1SoundEffect::MenuOptionPopup, nullptr);
      break;
    }

    currentObject.goalFrame = currentObject.openFrame;
    currentObject.animDirection = 1_frame;
    return create<ApplyItemTransformMenuState>();
  }

  if(presenter.getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Forward)
     && display.currentRingIndex > 0)
  {
    return create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Down,
                                        create<SwitchRingMenuState>(display.currentRingIndex - 1, false));
  }
  else if(presenter.getInputHandler().getInputState().zMovement.justChangedTo(hid::AxisMovement::Backward)
          && display.currentRingIndex + 1 < display.rings.size())
  {
    return create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Up,
                                        create<SwitchRingMenuState>(display.currentRingIndex + 1, false));
  }

  return nullptr;
}

void IdleRingMenuState::handleObject(ui::Ui& /*ui*/,
                                     engine::world::World& world,
                                     MenuDisplay& display,
                                     MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    idleRotation(world, object, display.getCurrentRing().list.size() == 1);
  }
  else
  {
    zeroRotation(object, 256_au);
  }
}
} // namespace menu
