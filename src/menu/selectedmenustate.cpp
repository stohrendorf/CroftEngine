#include "selectedmenustate.h"

#include "deflateringmenustate.h"
#include "deselectingmenustate.h"
#include "donemenustate.h"
#include "engine/items_tr1.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "finishitemanimationmenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "passportmenustate.h"
#include "rendersettingsmenustate.h"
#include "resetitemtransformmenustate.h"
#include "util.h"

namespace menu
{
std::unique_ptr<MenuState> SelectedMenuState::onFrame(engine::World& world, MenuDisplay& display)
{
  auto& currentObject = display.getCurrentRing().getSelectedObject();
  if(currentObject.type == engine::TR1ItemId::PassportClosed)
    return create<PassportMenuState>(display.mode, display.allowSave);
  else if(currentObject.type == engine::TR1ItemId::Sunglasses)
    return create<RenderSettingsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())),
      world.getEngine());

  if(currentObject.selectedRotationY == currentObject.rotationY && currentObject.animate())
    return nullptr;

  const bool autoSelect = display.doOptions(world, currentObject);
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
  {
    if(display.rings.size() > 1 || !display.allowMenuClose)
    {
      return create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>()));
    }
    else
    {
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeflateRingMenuState>(create<DoneMenuState>(MenuResult::Closed))));
    }
  }
  else if(autoSelect || world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Action))
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
    else
    {
      const auto result
        = currentObject.type == engine::TR1ItemId::LarasHomePolaroid ? MenuResult::LaraHome : MenuResult::Closed;
      return create<FinishItemAnimationMenuState>(
        create<ResetItemTransformMenuState>(create<DeflateRingMenuState>(create<DoneMenuState>(result))));
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
} // namespace menu
