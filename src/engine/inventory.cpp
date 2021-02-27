#include "inventory.h"

#include "engine/player.h"
#include "engine/world.h"
#include "items_tr1.h"
#include "objects/laraobject.h"
#include "serialization/map.h"
#include "serialization/serialization.h"

#include <boost/log/trivial.hpp>

namespace engine
{
void Inventory::put(const core::TypeId id, const size_t quantity)
{
  BOOST_LOG_TRIVIAL(debug) << "Object " << toString(id.get_as<TR1ItemId>()) << " added to inventory";

  auto addWeapon = [this](Ammo& ammo, size_t n) {
    // convert existing ammo clips to ammo
    ammo.addClips(count(ammo.ammoType) + n);
    m_inventory.erase(ammo.ammoType);
    m_inventory[ammo.weaponType] = 1;
  };

  auto addAmmoClips = [this](Ammo& ammo, size_t n) {
    if(count(ammo.weaponType) > 0)
      ammo.addClips(n);
    else
      m_inventory[ammo.ammoType] += n;
  };

  switch(id.get_as<TR1ItemId>())
  {
  case TR1ItemId::PistolsSprite: [[fallthrough]];
  case TR1ItemId::Pistols: m_inventory[TR1ItemId::Pistols] = 1; break;
  case TR1ItemId::ShotgunSprite: [[fallthrough]];
  case TR1ItemId::Shotgun:
    addWeapon(m_shotgunAmmo, quantity);
    // TODO replaceItems( ShotgunSprite, ShotgunAmmoSprite );
    break;
  case TR1ItemId::MagnumsSprite: [[fallthrough]];
  case TR1ItemId::Magnums:
    addWeapon(m_magnumsAmmo, quantity);
    // TODO replaceItems( MagnumsSprite, MagnumAmmoSprite );
    break;
  case TR1ItemId::UzisSprite: [[fallthrough]];
  case TR1ItemId::Uzis:
    addWeapon(m_uzisAmmo, quantity);
    // TODO replaceItems( UzisSprite, UziAmmoSprite );
    break;
  case TR1ItemId::ShotgunAmmoSprite: [[fallthrough]];
  case TR1ItemId::ShotgunAmmo: addAmmoClips(m_shotgunAmmo, quantity); break;
  case TR1ItemId::MagnumAmmoSprite: [[fallthrough]];
  case TR1ItemId::MagnumAmmo: addAmmoClips(m_magnumsAmmo, quantity); break;
  case TR1ItemId::UziAmmoSprite: [[fallthrough]];
  case TR1ItemId::UziAmmo: addAmmoClips(m_uzisAmmo, quantity); break;
  case TR1ItemId::SmallMedipackSprite: [[fallthrough]];
  case TR1ItemId::SmallMedipack: m_inventory[TR1ItemId::SmallMedipack] += quantity; break;
  case TR1ItemId::LargeMedipackSprite: [[fallthrough]];
  case TR1ItemId::LargeMedipack: m_inventory[TR1ItemId::LargeMedipack] += quantity; break;
  case TR1ItemId::Puzzle1Sprite: [[fallthrough]];
  case TR1ItemId::Puzzle1: m_inventory[TR1ItemId::Puzzle1] += quantity; break;
  case TR1ItemId::Puzzle2Sprite: [[fallthrough]];
  case TR1ItemId::Puzzle2: m_inventory[TR1ItemId::Puzzle2] += quantity; break;
  case TR1ItemId::Puzzle3Sprite: [[fallthrough]];
  case TR1ItemId::Puzzle3: m_inventory[TR1ItemId::Puzzle3] += quantity; break;
  case TR1ItemId::Puzzle4Sprite: [[fallthrough]];
  case TR1ItemId::Puzzle4: m_inventory[TR1ItemId::Puzzle4] += quantity; break;
  case TR1ItemId::LeadBarSprite: [[fallthrough]];
  case TR1ItemId::LeadBar: m_inventory[TR1ItemId::LeadBar] += quantity; break;
  case TR1ItemId::Key1Sprite: [[fallthrough]];
  case TR1ItemId::Key1: m_inventory[TR1ItemId::Key1] += quantity; break;
  case TR1ItemId::Key2Sprite: [[fallthrough]];
  case TR1ItemId::Key2: m_inventory[TR1ItemId::Key2] += quantity; break;
  case TR1ItemId::Key3Sprite: [[fallthrough]];
  case TR1ItemId::Key3: m_inventory[TR1ItemId::Key3] += quantity; break;
  case TR1ItemId::Key4Sprite: [[fallthrough]];
  case TR1ItemId::Key4: m_inventory[TR1ItemId::Key4] += quantity; break;
  case TR1ItemId::Item141: [[fallthrough]];
  case TR1ItemId::Item148: m_inventory[TR1ItemId::Item148] += quantity; break;
  case TR1ItemId::Item142: [[fallthrough]];
  case TR1ItemId::Item149: m_inventory[TR1ItemId::Item149] += quantity; break;
  case TR1ItemId::ScionPiece1: [[fallthrough]];
  case TR1ItemId::ScionPiece2: [[fallthrough]];
  case TR1ItemId::ScionPiece5: m_inventory[TR1ItemId::ScionPiece5] += quantity; break;
  default:
    BOOST_LOG_TRIVIAL(warning) << "Cannot add object " << toString(id.get_as<TR1ItemId>()) << " to inventory";
    return;
  }
}

bool Inventory::tryUse(objects::LaraObject& lara, const TR1ItemId id)
{
  auto tryUseWeapon = [this, &lara](TR1ItemId weapon, WeaponId weaponId) -> bool {
    if(count(weapon) == 0)
      return false;

    lara.getWorld().getPlayer().requestedGunType = weaponId;
    if(lara.getHandStatus() == objects::HandStatus::None
       && lara.getWorld().getPlayer().gunType == lara.getWorld().getPlayer().requestedGunType)
    {
      lara.getWorld().getPlayer().gunType = WeaponId::None;
    }

    return true;
  };

  auto tryUseMediPack = [this, &lara](TR1ItemId mediPack, const core::Health& health) -> bool {
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
    return tryUseWeapon(TR1ItemId::Shotgun, WeaponId::Shotgun);
  }
  else if(id == TR1ItemId::Pistols || id == TR1ItemId::PistolsSprite)
  {
    return tryUseWeapon(TR1ItemId::Pistols, WeaponId::Pistols);
  }
  else if(id == TR1ItemId::Magnums || id == TR1ItemId::MagnumsSprite)
  {
    return tryUseWeapon(TR1ItemId::Magnums, WeaponId::Magnums);
  }
  else if(id == TR1ItemId::Uzis || id == TR1ItemId::UzisSprite)
  {
    return tryUseWeapon(TR1ItemId::Uzis, WeaponId::Uzis);
  }
  else if(id == TR1ItemId::LargeMedipack || id == TR1ItemId::LargeMedipackSprite)
  {
    return tryUseMediPack(TR1ItemId::LargeMedipack, core::LaraHealth);
  }
  else if(id == TR1ItemId::SmallMedipack || id == TR1ItemId::SmallMedipackSprite)
  {
    return tryUseMediPack(TR1ItemId::LargeMedipack, core::LaraHealth / 2);
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

void Inventory::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("inventory", m_inventory),
      S_NV("pistolsAmmo", m_pistolsAmmo),
      S_NV("magnumsAmmo", m_magnumsAmmo),
      S_NV("uzisAmmo", m_uzisAmmo),
      S_NV("shotgunAmmo", m_shotgunAmmo));
}

void Ammo::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("ammo", ammo), S_NV("hits", hits), S_NV("misses", misses));
}
} // namespace engine
