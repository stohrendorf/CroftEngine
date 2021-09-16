#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "engine/weapontype.h"
#include "inventory.h"
#include "serialization/serialization_fwd.h"

#include <chrono>
#include <cstddef>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine::world
{
class World;
}

namespace engine
{
class Player
{
public:
  explicit Player() = default;

  [[nodiscard]] auto& getInventory()
  {
    return m_inventory;
  }

  [[nodiscard]] const auto& getInventory() const
  {
    return m_inventory;
  }

  void serialize(const serialization::Serializer<world::World>& ser);

  core::Health laraHealth{core::LaraHealth};
  WeaponType selectedWeaponType = WeaponType::None;
  WeaponType requestedWeaponType = WeaponType::None;
  size_t pickups = 0;
  size_t kills = 0;
  size_t secrets = 0;
  std::chrono::milliseconds timeSpent{0};

  void resetStats()
  {
    pickups = 0;
    kills = 0;
    secrets = 0;
    timeSpent = std::chrono::milliseconds{0};
  }

private:
  Inventory m_inventory{};
};
} // namespace engine
