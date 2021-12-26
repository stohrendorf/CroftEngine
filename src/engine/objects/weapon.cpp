#include "weapon.h"

#include "core/magic.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <unordered_map>

namespace engine::objects
{
const Weapon& Weapon::get(WeaponType weaponType)
{
  static const std::unordered_map<WeaponType, Weapon> weapons{{WeaponType::None, Weapon{}},
                                                              {WeaponType::Pistols,
                                                               Weapon{WeaponType::Pistols,
                                                                      {{-60_deg, +60_deg}, {-60_deg, +60_deg}},
                                                                      {{-170_deg, +60_deg}, {-80_deg, +80_deg}},
                                                                      {{-60_deg, +170_deg}, {-80_deg, +80_deg}},
                                                                      +10_deg / 1_frame,
                                                                      +4_deg,
                                                                      650_len,
                                                                      1_hp,
                                                                      core::SectorSize * 8,
                                                                      9_frame,
                                                                      3_frame,
                                                                      TR1SoundEffect::LaraShootPistols}},
                                                              {WeaponType::Magnums,
                                                               Weapon{WeaponType::Magnums,
                                                                      {{-60_deg, +60_deg}, {-60_deg, +60_deg}},
                                                                      {{-170_deg, +60_deg}, {-80_deg, +80_deg}},
                                                                      {{-60_deg, +170_deg}, {-80_deg, +80_deg}},
                                                                      +10_deg / 1_frame,
                                                                      +4_deg,
                                                                      650_len,
                                                                      2_hp,
                                                                      core::SectorSize * 8,
                                                                      9_frame,
                                                                      3_frame,
                                                                      TR1SoundEffect::CowboyShoot}},
                                                              {WeaponType::Uzis,
                                                               Weapon{WeaponType::Uzis,
                                                                      {{-60_deg, +60_deg}, {-60_deg, +60_deg}},
                                                                      {{-170_deg, +60_deg}, {-80_deg, +80_deg}},
                                                                      {{-60_deg, +170_deg}, {-80_deg, +80_deg}},
                                                                      +10_deg / 1_frame,
                                                                      +4_deg,
                                                                      650_len,
                                                                      1_hp,
                                                                      core::SectorSize * 8,
                                                                      3_frame,
                                                                      2_frame,
                                                                      TR1SoundEffect::LaraShootUzis}},
                                                              {WeaponType::Shotgun,
                                                               Weapon{WeaponType::Shotgun,
                                                                      {{-60_deg, +60_deg}, {-55_deg, +55_deg}},
                                                                      {{-80_deg, +80_deg}, {-65_deg, +65_deg}},
                                                                      {{-80_deg, +80_deg}, {-65_deg, +65_deg}},
                                                                      +10_deg / 1_frame,
                                                                      0_deg,
                                                                      500_len,
                                                                      4_hp,
                                                                      core::SectorSize * 8,
                                                                      9_frame,
                                                                      3_frame,
                                                                      TR1SoundEffect::LaraShootShotgun}}};
  auto it = weapons.find(weaponType);
  if(it == weapons.end())
    BOOST_THROW_EXCEPTION(std::domain_error("invalid weapon type"));
  return it->second;
}
} // namespace engine::objects
