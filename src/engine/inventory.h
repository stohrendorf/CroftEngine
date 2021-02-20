#pragma once

#include "core/id.h"
#include "weaponid.h"

#include <boost/throw_exception.hpp>
#include <gsl-lite.hpp>
#include <map>
#include <stdexcept>

namespace engine
{
namespace objects
{
class LaraObject;
}

class World;

struct Ammo
{
  size_t ammo = 0;
  uint32_t hits = 0;
  uint32_t misses = 0;

  void serialize(const serialization::Serializer<World>& ser);
};

class Inventory
{
private:
  std::map<TR1ItemId, size_t> m_inventory;

  Ammo m_pistolsAmmo;
  Ammo m_magnumsAmmo;
  Ammo m_uzisAmmo;
  Ammo m_shotgunAmmo;

public:
  explicit Inventory() = default;

  void serialize(const serialization::Serializer<World>& ser);

  void put(core::TypeId id, size_t quantity = 1);

  bool tryTake(TR1ItemId id, size_t quantity = 1);

  [[nodiscard]] size_t count(const TR1ItemId id) const
  {
    const auto it = m_inventory.find(id);
    if(it == m_inventory.end())
      return 0;

    return it->second;
  }

  [[nodiscard]] bool any() const
  {
    return std::find_if(m_inventory.begin(), m_inventory.end(), [](const auto& kv) { return kv.second != 0; })
           != m_inventory.end();
  }

  void clear()
  {
    m_inventory.clear();
  }

  void drop(TR1ItemId id)
  {
    m_inventory.erase(id);
  }

  bool tryUse(objects::LaraObject& lara, TR1ItemId id);

  gsl::not_null<Ammo*> getAmmo(WeaponId weaponId)
  {
    m_pistolsAmmo.ammo = 1000;

    switch(weaponId)
    {
    case WeaponId::Pistols: return &m_pistolsAmmo; break;
    case WeaponId::Magnums: return &m_magnumsAmmo; break;
    case WeaponId::Uzis: return &m_uzisAmmo; break;
    case WeaponId::Shotgun: return &m_shotgunAmmo; break;
    default: BOOST_THROW_EXCEPTION(std::domain_error("weaponId"));
    }
  }
};
} // namespace engine
