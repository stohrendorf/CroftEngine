#include "player.h"

#include "engine/objectmanager.h"
#include "objects/laraobject.h"
#include "serialization/chrono.h"
#include "serialization/optional_value.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "world/world.h"

#include <exception>
#include <memory>
#include <utility>

namespace engine
{
void Player::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("inventory", m_inventory),
      S_NV("laraHealth", laraHealth),
      S_NV("selectedWeaponType", selectedWeaponType),
      S_NV("requestedWeaponType", requestedWeaponType),
      S_NV("pickups", pickups),
      S_NVO("pickupsTotal", pickupsTotal),
      S_NV("kills", kills),
      S_NVO("killsTotal", killsTotal),
      S_NV("secrets", secrets),
      S_NVO("secretsTotal", secretsTotal),
      S_NVO("smallMedipacks", smallMedipacks),
      S_NVO("smallMedipacksTotal", smallMedipacksTotal),
      S_NVO("largeMedipacks", largeMedipacks),
      S_NVO("largeMedipacksTotal", largeMedipacksTotal),
      S_NV("timeSpent", timeSpent),
      S_NVO("timeSpentTotal", timeSpentTotal),
      S_NVO("usedCheats", usedCheats));

  if(ser.loading && ser.context.getObjectManager().getLaraPtr() != nullptr)
    ser.lazy(
      [](const serialization::Serializer<world::World>& ser)
      {
        ser.context.getObjectManager().getLara().initWeaponAnimData();
      });
}

void Player::accumulateStats()
{
  pickupsTotal += std::exchange(pickups, 0);
  killsTotal += std::exchange(kills, 0);
  secretsTotal += std::exchange(secrets, 0);
  smallMedipacksTotal += std::exchange(smallMedipacks, 0);
  largeMedipacksTotal += std::exchange(largeMedipacks, 0);
  timeSpentTotal += std::exchange(timeSpent, 0_frame);
  for(auto ammoType : {WeaponType::Pistols, WeaponType::Shotgun, WeaponType::Uzis, WeaponType::Magnums})
  {
    auto& ammo = getInventory().getAmmo(ammoType);
    ammo.hitsTotal += std::exchange(ammo.hits, 0);
    ammo.missesTotal += std::exchange(ammo.misses, 0);
  }
}
} // namespace engine
