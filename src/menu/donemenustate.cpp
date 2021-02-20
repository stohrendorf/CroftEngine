#include "donemenustate.h"

#include "engine/engine.h"
#include "engine/items_tr1.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
void DoneMenuState::handleObject(ui::Ui& ui, engine::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    display.updateMenuObjectDescription(ui, world, object);
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(world, object);
  }
}

std::unique_ptr<MenuState> DoneMenuState::onFrame(ui::Ui& /*ui*/, engine::World& world, MenuDisplay& display)
{
  display.result = m_result;

  switch(display.inventoryChosen.value_or(engine::TR1ItemId::Lara))
  {
  case engine::TR1ItemId::Pistols:
    world.getEngine().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Pistols);
    break;
  case engine::TR1ItemId::Shotgun:
    world.getEngine().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Shotgun);
    break;
  case engine::TR1ItemId::Magnums:
    world.getEngine().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Magnums);
    break;
  case engine::TR1ItemId::Uzis:
    world.getEngine().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Uzis);
    break;
  case engine::TR1ItemId::SmallMedipack:
    world.getEngine().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::SmallMedipack);
    break;
  case engine::TR1ItemId::LargeMedipack:
    world.getEngine().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::LargeMedipack);
    break;
  default: break;
  }

  return nullptr;
}
} // namespace menu
