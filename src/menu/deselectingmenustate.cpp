#include "deselectingmenustate.h"

#include "engine/audioengine.h"
#include "engine/world/world.h"
#include "idleringmenustate.h"
#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
std::unique_ptr<MenuState>
  DeselectingMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& display)
{
  display.updateRingTitle();
  return create<IdleRingMenuState>(false);
}

void DeselectingMenuState::handleObject(ui::Ui& /*ui*/,
                                        engine::world::World& /*world*/,
                                        MenuDisplay& display,
                                        MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}

DeselectingMenuState::DeselectingMenuState(const std::shared_ptr<MenuRingTransform>& ringTransform)
    : MenuState{ringTransform}
{
}
} // namespace menu
