#include "donemenustate.h"

#include "engine/items_tr1.h"
#include "engine/world.h"
#include "menudisplay.h"
#include "menuring.h"
#include "util.h"

namespace menu
{
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
} // namespace menu
