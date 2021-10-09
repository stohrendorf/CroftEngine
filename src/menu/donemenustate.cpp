#include "donemenustate.h"

#include "core/angle.h"
#include "engine/inventory.h"
#include "engine/items_tr1.h"
#include "engine/objectmanager.h"
#include "engine/player.h"
#include "engine/world/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

#include <optional>

namespace menu
{
class MenuState;

void DoneMenuState::handleObject(ui::Ui& /*ui*/, engine::world::World& world, MenuDisplay& display, MenuObject& object)
{
  if(&object == &display.getCurrentRing().getSelectedObject())
  {
    zeroRotation(object, 256_au);
  }
  else
  {
    idleRotation(world, object, false);
  }
}

std::unique_ptr<MenuState> DoneMenuState::onFrame(ui::Ui& /*ui*/, engine::world::World& world, MenuDisplay& display)
{
  display.result = m_result;

  switch(display.inventoryChosen.value_or(engine::TR1ItemId::Lara))
  {
  case engine::TR1ItemId::Pistols:
    world.getPlayer().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Pistols);
    break;
  case engine::TR1ItemId::Shotgun:
    world.getPlayer().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Shotgun);
    break;
  case engine::TR1ItemId::Magnums:
    world.getPlayer().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Magnums);
    break;
  case engine::TR1ItemId::Uzis:
    world.getPlayer().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::Uzis);
    break;
  case engine::TR1ItemId::SmallMedipack:
    world.getPlayer().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::SmallMedipack);
    break;
  case engine::TR1ItemId::LargeMedipack:
    world.getPlayer().getInventory().tryUse(world.getObjectManager().getLara(), engine::TR1ItemId::LargeMedipack);
    break;
  default:
    break;
  }

  return nullptr;
}
} // namespace menu
