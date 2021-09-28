#pragma once

#include "core/id.h"
#include "items_tr1.h"
#include "serialization/serialization_fwd.h"
#include "weapontype.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <stdexcept>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::world
{
class World;
}

namespace engine
{
namespace objects
{
class LaraObject;
}

struct Ammo
{
  const size_t roundsPerClip;
  const size_t roundsPerShot;
  const TR1ItemId ammoType;
  const TR1ItemId weaponType;
  size_t ammo = 0;
  uint32_t hits = 0;
  uint32_t misses = 0;

  void addClips(size_t n)
  {
    ammo += roundsPerClip * n;
  }

  void serialize(const serialization::Serializer<world::World>& ser);
};

class Inventory
{
private:
  std::map<TR1ItemId, size_t> m_inventory;

  Ammo m_pistolsAmmo{1, 1, TR1ItemId::Lara, TR1ItemId::Pistols};
  Ammo m_magnumsAmmo{50, 1, TR1ItemId::MagnumAmmo, TR1ItemId::Magnums};
  Ammo m_uzisAmmo{100, 1, TR1ItemId::UziAmmo, TR1ItemId::Uzis};
  Ammo m_shotgunAmmo{12, 6, TR1ItemId::ShotgunAmmo, TR1ItemId::Shotgun};

public:
  explicit Inventory() = default;

  void serialize(const serialization::Serializer<world::World>& ser);

  void put(const core::TypeId& id, size_t quantity = 1);

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

  Ammo& getAmmo(WeaponType weaponType)
  {
    m_pistolsAmmo.ammo = 1000;

    switch(weaponType)
    {
    case WeaponType::Pistols: return m_pistolsAmmo;
    case WeaponType::Magnums: return m_magnumsAmmo;
    case WeaponType::Uzis: return m_uzisAmmo;
    case WeaponType::Shotgun: return m_shotgunAmmo;
    default: BOOST_THROW_EXCEPTION(std::domain_error("weaponType"));
    }
  }

  void fillAllAmmo()
  {
    m_pistolsAmmo.ammo = 1000;
    m_magnumsAmmo.ammo = 1000;
    m_uzisAmmo.ammo = 1000;
    m_shotgunAmmo.ammo = 1000;
  }
};
} // namespace engine
