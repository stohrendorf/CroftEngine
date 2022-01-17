#include "selectedmenustate.h"

#include "audiosettingsmenustate.h"
#include "controlsmenustate.h"
#include "core/units.h"
#include "deflateringmenustate.h"
#include "deselectingmenustate.h"
#include "donemenustate.h"
#include "engine/items_tr1.h"
#include "engine/presenter.h"
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
#include "rendersettingsmenustate.h"
#include "resetitemtransformmenustate.h"
#include "ui/text.h"
#include "ui/ui.h"
#include "util.h"

#include <glm/vec2.hpp>
#include <optional>
#include <vector>

namespace menu
{
std::unique_ptr<MenuState> SelectedMenuState::onFrame(ui::Ui& ui, engine::world::World& world, MenuDisplay& display)
{
  auto& currentObject = display.getCurrentRing().getSelectedObject();
  if(currentObject.type == engine::TR1ItemId::PassportClosed)
    return create<PassportMenuState>(display.mode, display.allowSave);
  else if(currentObject.type == engine::TR1ItemId::Sunglasses)
    return create<RenderSettingsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())),
      world.getEngine());
  else if(currentObject.type == engine::TR1ItemId::DirectionKeys)
    return create<ControlsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())), world);
  else if(currentObject.type == engine::TR1ItemId::CassettePlayer)
    return create<AudioSettingsMenuState>(
      create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(create<DeselectingMenuState>())),
      display);

  if(currentObject.selectedRotationY == currentObject.rotationY && currentObject.animate())
    return nullptr;

  const bool autoSelect = MenuDisplay::doOptions(world, currentObject);
  if(world.getPresenter().getInputHandler().hasDebouncedAction(hid::Action::Menu))
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
      return create<FinishItemAnimationMenuState>(create<ResetItemTransformMenuState>(
        create<DeflateRingMenuState>(DeflateRingMenuState::Direction::Backpack, create<DoneMenuState>(result))));
    }
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
