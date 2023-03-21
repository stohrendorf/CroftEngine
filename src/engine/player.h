#pragma once

#include "core/magic.h"
#include "core/units.h"
#include "engine/weapontype.h"
#include "inventory.h"
#include "serialization/serialization_fwd.h"

#include <cstddef>

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

  [[nodiscard]] auto& getInventory() noexcept
  {
    return m_inventory;
  }

  [[nodiscard]] const auto& getInventory() const noexcept
  {
    return m_inventory;
  }

  void serialize(const serialization::Serializer<world::World>& ser) const;
  void deserialize(const serialization::Deserializer<world::World>& ser);

  core::Health laraHealth{core::LaraHealth};
  WeaponType selectedWeaponType = WeaponType::None;
  WeaponType requestedWeaponType = WeaponType::None;
  size_t pickups = 0;
  size_t pickupsTotal = 0;
  size_t kills = 0;
  size_t killsTotal = 0;
  size_t secrets = 0;
  size_t secretsTotal = 0;
  size_t smallMedipacks = 0;
  size_t smallMedipacksTotal = 0;
  size_t largeMedipacks = 0;
  size_t largeMedipacksTotal = 0;
  core::Frame timeSpent = 0_frame;
  core::Frame timeSpentTotal = 0_frame;
  bool usedCheats = false;

  void accumulateStats();

private:
  Inventory m_inventory{};
};
} // namespace engine
