#include "player.h"

#include "engine/objectmanager.h"
#include "inventory.h"
#include "objects/laraobject.h"
#include "serialization/optional_value.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "world/world.h"

#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <utility>

namespace engine
{
void Player::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("inventory", m_inventory),
      S_NV("laraHealth", laraHealth),
      S_NV("selectedWeaponType", selectedWeaponType),
      S_NV("requestedWeaponType", requestedWeaponType),
      S_NV("pickups", pickups),
      S_NV("pickupsTotal", pickupsTotal),
      S_NV("kills", kills),
      S_NV("killsTotal", killsTotal),
      S_NV("secrets", secrets),
      S_NV("secretsTotal", secretsTotal),
      S_NV("smallMedipacks", smallMedipacks),
      S_NV("smallMedipacksTotal", smallMedipacksTotal),
      S_NV("largeMedipacks", largeMedipacks),
      S_NV("largeMedipacksTotal", largeMedipacksTotal),
      S_NV("timeSpent", timeSpent),
      S_NV("timeSpentTotal", timeSpentTotal),
      S_NV("usedCheats", usedCheats));
}

void Player::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("inventory", m_inventory),
      S_NV("laraHealth", laraHealth),
      S_NV("selectedWeaponType", selectedWeaponType),
      S_NV("requestedWeaponType", requestedWeaponType),
      S_NV("pickups", pickups),
      S_NVO("pickupsTotal", std::ref(pickupsTotal)),
      S_NV("kills", kills),
      S_NVO("killsTotal", std::ref(killsTotal)),
      S_NV("secrets", secrets),
      S_NVO("secretsTotal", std::ref(secretsTotal)),
      S_NVO("smallMedipacks", std::ref(smallMedipacks)),
      S_NVO("smallMedipacksTotal", std::ref(smallMedipacksTotal)),
      S_NVO("largeMedipacks", std::ref(largeMedipacks)),
      S_NVO("largeMedipacksTotal", std::ref(largeMedipacksTotal)),
      S_NV("timeSpent", timeSpent),
      S_NVO("timeSpentTotal", std::ref(timeSpentTotal)),
      S_NVO("usedCheats", std::ref(usedCheats)));

  if(ser.context->getObjectManager().getLaraPtr() != nullptr)
    ser << [](const serialization::Deserializer<world::World>& ser)
    {
      ser.context->getObjectManager().getLara().initWeaponAnimData();
    };
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
