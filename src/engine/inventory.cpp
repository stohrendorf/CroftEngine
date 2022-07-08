#include "inventory.h"

#include "core/magic.h"
#include "core/units.h"
#include "engine/objectmanager.h"
#include "items_tr1.h"
#include "objects/laraobject.h"
#include "objects/objectstate.h"
#include "player.h"
#include "qs/qs.h"
#include "serialization/map.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"
#include "soundeffects_tr1.h"
#include "weapontype.h"
#include "world/world.h"

#include <boost/log/trivial.hpp>
#include <exception>

namespace engine
{
size_t Inventory::put(const core::TypeId& id, world::World* world, const size_t quantity)
{
  BOOST_LOG_TRIVIAL(debug) << "Object " << toString(id.get_as<TR1ItemId>()) << " added to inventory";

  auto addWeapon = [this](const Ammo& ammo)
  {
    m_inventory[ammo.weaponType] = 1;
  };

  switch(id.get_as<TR1ItemId>())
  {
  case TR1ItemId::PistolsSprite:
    [[fallthrough]];
  case TR1ItemId::Pistols:
    m_inventory[TR1ItemId::Pistols] = 1;
    return 1;
  case TR1ItemId::ShotgunSprite:
  case TR1ItemId::Shotgun:
    addWeapon(m_shotgunAmmo);
    if(world != nullptr)
      world->getObjectManager().replaceItems(TR1ItemId::ShotgunSprite, TR1ItemId::ShotgunAmmoSprite, *world);
    return m_shotgunAmmo.shots;
  case TR1ItemId::MagnumsSprite:
  case TR1ItemId::Magnums:
    addWeapon(m_magnumsAmmo);
    if(world != nullptr)
      world->getObjectManager().replaceItems(TR1ItemId::MagnumsSprite, TR1ItemId::MagnumAmmoSprite, *world);
    return m_magnumsAmmo.shots;
  case TR1ItemId::UzisSprite:
  case TR1ItemId::Uzis:
    addWeapon(m_uzisAmmo);
    if(world != nullptr)
      world->getObjectManager().replaceItems(TR1ItemId::UzisSprite, TR1ItemId::UziAmmoSprite, *world);
    return m_uzisAmmo.shots;
  case TR1ItemId::ShotgunAmmoSprite:
  case TR1ItemId::ShotgunAmmo:
    m_shotgunAmmo.addClips(quantity);
    return m_shotgunAmmo.shots;
  case TR1ItemId::MagnumAmmoSprite:
  case TR1ItemId::MagnumAmmo:
    m_magnumsAmmo.addClips(quantity);
    return m_magnumsAmmo.shots;
  case TR1ItemId::UziAmmoSprite:
  case TR1ItemId::UziAmmo:
    m_uzisAmmo.addClips(quantity);
    return m_uzisAmmo.shots;
  case TR1ItemId::SmallMedipackSprite:
  case TR1ItemId::SmallMedipack:
    return m_inventory[TR1ItemId::SmallMedipack] += quantity;
  case TR1ItemId::LargeMedipackSprite:
  case TR1ItemId::LargeMedipack:
    return m_inventory[TR1ItemId::LargeMedipack] += quantity;
  case TR1ItemId::Puzzle1Sprite:
  case TR1ItemId::Puzzle1:
    return m_inventory[TR1ItemId::Puzzle1] += quantity;
  case TR1ItemId::Puzzle2Sprite:
  case TR1ItemId::Puzzle2:
    return m_inventory[TR1ItemId::Puzzle2] += quantity;
  case TR1ItemId::Puzzle3Sprite:
  case TR1ItemId::Puzzle3:
    return m_inventory[TR1ItemId::Puzzle3] += quantity;
  case TR1ItemId::Puzzle4Sprite:
  case TR1ItemId::Puzzle4:
    return m_inventory[TR1ItemId::Puzzle4] += quantity;
  case TR1ItemId::LeadBarSprite:
  case TR1ItemId::LeadBar:
    return m_inventory[TR1ItemId::LeadBar] += quantity;
  case TR1ItemId::Key1Sprite:
  case TR1ItemId::Key1:
    return m_inventory[TR1ItemId::Key1] += quantity;
  case TR1ItemId::Key2Sprite:
  case TR1ItemId::Key2:
    return m_inventory[TR1ItemId::Key2] += quantity;
  case TR1ItemId::Key3Sprite:
  case TR1ItemId::Key3:
    return m_inventory[TR1ItemId::Key3] += quantity;
  case TR1ItemId::Key4Sprite:
  case TR1ItemId::Key4:
    return m_inventory[TR1ItemId::Key4] += quantity;
  case TR1ItemId::Item141:
  case TR1ItemId::Item148:
    return m_inventory[TR1ItemId::Item148] += quantity;
  case TR1ItemId::Item142:
  case TR1ItemId::Item149:
    return m_inventory[TR1ItemId::Item149] += quantity;
  case TR1ItemId::ScionPiece1:
  case TR1ItemId::ScionPiece2:
  case TR1ItemId::ScionPiece5:
    return m_inventory[TR1ItemId::ScionPiece5] += quantity;
  default:
    BOOST_LOG_TRIVIAL(warning) << "Cannot add object " << toString(id.get_as<TR1ItemId>()) << " to inventory";
    return 0;
  }
}

bool Inventory::tryUse(objects::LaraObject& lara, const TR1ItemId id)
{
  auto tryUseWeapon = [this, &lara](TR1ItemId weapon, WeaponType weaponType) -> bool
  {
    if(count(weapon) == 0)
      return false;

    auto& player = lara.getWorld().getPlayer();
    player.requestedWeaponType = weaponType;
    if(player.selectedWeaponType == player.requestedWeaponType)
    {
      if(lara.getHandStatus() == objects::HandStatus::None)
      {
        player.selectedWeaponType = WeaponType::None;
      }
      else if(lara.getHandStatus() == objects::HandStatus::Combat)
      {
        lara.setHandStatus(engine::objects::HandStatus::Holster);
        lara.updateLarasWeaponsStatus();
      }
    }

    return true;
  };

  auto tryUseMediPack = [this, &lara](TR1ItemId mediPack, const core::Health& health) -> bool
  {
    if(count(mediPack) == 0)
      return false;

    if(lara.isDead() || lara.m_state.health >= core::LaraHealth)
    {
      return false;
    }

    lara.m_state.health = std::min(lara.m_state.health + health, core::LaraHealth);
    tryTake(mediPack);
    lara.playSoundEffect(TR1SoundEffect::LaraSigh);
    return true;
  };

  if(id == TR1ItemId::Shotgun || id == TR1ItemId::ShotgunSprite)
  {
    return tryUseWeapon(TR1ItemId::Shotgun, WeaponType::Shotgun);
  }
  else if(id == TR1ItemId::Pistols || id == TR1ItemId::PistolsSprite)
  {
    return tryUseWeapon(TR1ItemId::Pistols, WeaponType::Pistols);
  }
  else if(id == TR1ItemId::Magnums || id == TR1ItemId::MagnumsSprite)
  {
    return tryUseWeapon(TR1ItemId::Magnums, WeaponType::Magnums);
  }
  else if(id == TR1ItemId::Uzis || id == TR1ItemId::UzisSprite)
  {
    return tryUseWeapon(TR1ItemId::Uzis, WeaponType::Uzis);
  }
  else if(id == TR1ItemId::LargeMedipack || id == TR1ItemId::LargeMedipackSprite)
  {
    return tryUseMediPack(TR1ItemId::LargeMedipack, core::LaraHealth);
  }
  else if(id == TR1ItemId::SmallMedipack || id == TR1ItemId::SmallMedipackSprite)
  {
    return tryUseMediPack(TR1ItemId::SmallMedipack, core::LaraHealth / 2);
  }

  return true;
}

bool Inventory::tryTake(const TR1ItemId id, const size_t quantity)
{
  BOOST_LOG_TRIVIAL(debug) << "Taking object " << toString(id) << " from inventory";

  const auto it = m_inventory.find(id);
  if(it == m_inventory.end())
    return false;

  if(it->second < quantity)
    return false;

  if(it->second == quantity)
    m_inventory.erase(it);
  else
    m_inventory[id] -= quantity;

  return true;
}

void Inventory::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("inventory", m_inventory),
      S_NV("pistolsAmmo", m_pistolsAmmo),
      S_NV("magnumsAmmo", m_magnumsAmmo),
      S_NV("uzisAmmo", m_uzisAmmo),
      S_NV("shotgunAmmo", m_shotgunAmmo));
}

void Ammo::serialize(const serialization::Serializer<world::World>& ser)
{
  size_t ammo = std::numeric_limits<size_t>::max();

  // TODO remove in 1.11
  if(ser.loading)
  {
    ser(S_NVO("ammo", ammo));
  }

  ser(S_NVO("shots", shots),
      S_NV("hits", hits),
      S_NVO("hitsTotal", hitsTotal),
      S_NV("misses", misses),
      S_NVO("missesTotal", missesTotal));

  // TODO remove in 1.11
  if(ser.loading && ammo != std::numeric_limits<size_t>::max())
  {
    shots = std::exchange(ammo, std::numeric_limits<size_t>::max()) / roundsPerShot;
  }
}
} // namespace engine
