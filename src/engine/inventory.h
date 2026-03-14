#pragma once

#include "core/id.h"
#include "items_tr1.h"
#include "serialization/serialization_fwd.h"
#include "weapontype.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>

namespace hid
{
class InputHandler;
}

namespace engine::world
{
class World;
} // namespace engine::world

namespace engine::objects
{
class LaraObject;
}

namespace engine
{
struct GameplayRules;

struct Ammo
{
  size_t shotsPerClip = 1;
  size_t roundsPerShot = 1;
  TR1ItemId weaponType{-1};
  char iconChar = 0;
  size_t shots = 0;
  uint32_t hits = 0;
  uint32_t hitsTotal = 0;
  uint32_t misses = 0;
  uint32_t missesTotal = 0;

  void addClips(const size_t n) noexcept
  {
    shots += shotsPerClip * n;
  }

  [[nodiscard]] size_t getClips() const noexcept
  {
    return shots / shotsPerClip;
  }

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  [[nodiscard]] auto getDisplayString() const
  {
    return std::to_string(shots) + " " + iconChar;
  }
};

class Inventory
{
  std::map<TR1ItemId, size_t> m_inventory;

  Ammo m_pistolsAmmo{1, 1, TR1ItemId::Pistols, '\0'};
  Ammo m_magnumsAmmo{50, 1, TR1ItemId::Magnums, 'B'};
  Ammo m_uzisAmmo{100, 1, TR1ItemId::Uzis, 'C'};
  Ammo m_shotgunAmmo{2, 6, TR1ItemId::Shotgun, 'A'};

public:
  explicit Inventory() = default;

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  size_t put(const core::TypeId& id, world::World* world, size_t quantity = 1);

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
    return std::ranges::find_if(m_inventory,
                                [](const auto& kv)
                                {
                                  return kv.second != 0;
                                })
           != m_inventory.end();
  }

  void clear() noexcept
  {
    m_inventory.clear();
  }

  void drop(const TR1ItemId id) noexcept
  {
    m_inventory.erase(id);
  }

  bool tryUse(objects::LaraObject& lara,
              TR1ItemId id,
              const GameplayRules& gameplayRules,
              const hid::InputHandler& inputHandler);

  [[nodiscard]] Ammo& getAmmo(const WeaponType weaponType)
  {
    m_pistolsAmmo.shots = 1000;

    switch(weaponType)
    {
    case WeaponType::Pistols:
      return m_pistolsAmmo;
    case WeaponType::Magnums:
      return m_magnumsAmmo;
    case WeaponType::Uzis:
      return m_uzisAmmo;
    case WeaponType::Shotgun:
      return m_shotgunAmmo;
    default:
      BOOST_THROW_EXCEPTION(std::domain_error("weaponType"));
    }
  }

  [[nodiscard]] const Ammo& getAmmo(const WeaponType weaponType) const
  {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<Inventory*>(this)->getAmmo(weaponType);
  }

  void fillAllAmmo() noexcept
  {
    m_pistolsAmmo.shots = 1000;
    m_magnumsAmmo.shots = 1000;
    m_uzisAmmo.shots = 1000;
    m_shotgunAmmo.shots = 1000;
  }
};
} // namespace engine
