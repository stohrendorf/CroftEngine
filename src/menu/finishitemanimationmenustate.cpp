#include "finishitemanimationmenustate.h"

#include "core/units.h"
#include "engine/items_tr1.h"
#include "menudisplay.h"
#include "menuobject.h"
#include "menuring.h"
#include "menustate.h"
#include "util.h"

#include <memory>
#include <utility>

namespace menu
{
std::unique_ptr<MenuState> FinishItemAnimationMenuState::tick(engine::world::World& world, MenuDisplay& display)
{
  auto& object = display.getCurrentRing().getSelectedObject();
  if(object.tick())
    return nullptr; // play full animation until its end

  if(object.type == engine::TR1ItemId::PassportOpening)
  {
    object.type = engine::TR1ItemId::PassportClosed;
    object.meshAnimFrame = 0_mframe;
    object.initModel(world, display.getLightsBuffer());
  }

  return std::move(m_next);
}

void FinishItemAnimationMenuState::constructUi(ui::Ui& /*ui*/, engine::world::World& /*world*/, MenuDisplay& /*display*/)
{
}

void FinishItemAnimationMenuState::handleObjectTick(engine::world::World& /*world*/,
                                                    MenuDisplay& display,
                                                    MenuObject& object)
{
  if(&object != &display.getCurrentRing().getSelectedObject())
    zeroRotation(object, 256_au);
  else
    rotateForSelection(object);
}
} // namespace menu
