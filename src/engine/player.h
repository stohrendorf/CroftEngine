#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "inventory.h"

namespace engine
{
class Player
{
public:
  explicit Player() = default;

  auto& getInventory()
  {
    return m_inventory;
  }

  const auto& getInventory() const
  {
    return m_inventory;
  }

  void serialize(const serialization::Serializer<World>& ser);

  core::Health laraHealth{core::LaraHealth};
  WeaponId gunType = WeaponId::None;
  WeaponId requestedGunType = WeaponId::None;

private:
  Inventory m_inventory{};
};
} // namespace engine
