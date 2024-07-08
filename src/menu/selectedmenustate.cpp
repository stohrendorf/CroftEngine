#include "selectedmenustate.h"

#include "audiosettingsmenustate.h"
#include "compassmenustate.h"
#include "controlsmenustate.h"
#include "core/units.h"
#include "deflateringmenustate.h"
#include "deselectingmenustate.h"
#include "donemenustate.h"
#include "engine/items_tr1.h"
#include "engine/objectmanager.h"
#include "engine/objects/laraobject.h"
#include "engine/presenter.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/world.h"
#include "finishitemanimationmenustate.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "menudisplay.h"
#include "menuobject.h"
#include "menuring.h"
#include "menustate.h"
#include "passportmenustate.h"
#include "qs/quantity.h"
#include "resetitemtransformmenustate.h"
#include "settingsmenustate.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util.h"

#include <memory>

namespace menu
{
std::unique_ptr<MenuState> SelectedMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  auto& currentObject = display.getCurrentRing().getSelectedObject();
  switch(currentObject.type)
  {
  case engine::TR1ItemId::PassportClosed:
    return create<PassportMenuState>(display.allowPassportExit, display.saveGamePageMode);
  case engine::TR1ItemId::Sunglasses:
    return create<SettingsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())),
      world.getEngine());
  case engine::TR1ItemId::DirectionKeys:
    return create<ControlsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())), world);
  case engine::TR1ItemId::CassettePlayer:
    return create<AudioSettingsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())));
  case engine::TR1ItemId::Compass:
    return create<FinishItemAnimationMenuState>(create<CompassMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())),
      world));
  default:
    break;
  }

  if(currentObject.selectedRotationY == currentObject.rotationY && currentObject.animate())
    return nullptr;

  const bool autoSelect = MenuDisplay::doOptions(world, currentObject);
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Return))
  {
    if(display.rings.size() > 1 || !display.allowMenuClose)
    {
      return create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>()));
    }
    else
    {
      return create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeflateRingMenuState>(
        DeflateRingMenuState::Direction::Backpack, create<DoneMenuState>(MenuResult::Closed))));
    }
  }
  else if(autoSelect || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::PrimaryInteraction))
  {
    display.inventoryChosen = currentObject.type;
    if(display.mode == InventoryMode::TitleMode
       && (currentObject.type == engine::TR1ItemId::Sunglasses
           || currentObject.type == engine::TR1ItemId::CassettePlayer
           || currentObject.type == engine::TR1ItemId::DirectionKeys
           || currentObject.type == engine::TR1ItemId::Flashlight))
    {
      return create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>()));
    }

    switch(currentObject.type)
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
      if(auto lara = world.getObjectManager().getLaraPtr())
        lara->playSoundEffect(engine::TR1SoundEffect::LaraNo);
      break;
    default:
      break;
    }
    const auto result
      = currentObject.type == engine::TR1ItemId::LarasHomePolaroid ? MenuResult::LaraHome : MenuResult::Closed;
    return create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(
      create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack, create<DoneMenuState>(result))));
  }

  if(m_itemTitle == nullptr)
    m_itemTitle = std::make_unique<ui::Text>(currentObject.name);
  m_itemTitle->draw(
    ui, world.getPresenter().getTrFont(), {(ui.getSize().x - m_itemTitle->getWidth()) / 2, ui.getSize().y - 16});

  return nullptr;
}

void SelectedMenuState::handleObject(ui::Ui& /*ui*/,
                                     engine::world::World& /*world*/,
                                     MenuDisplay& display,
                                     MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

SelectedMenuState::SelectedMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
    : MenuState{ringTransform}
{
}

SelectedMenuState::~SelectedMenuState() = default;
} // namespace menu
