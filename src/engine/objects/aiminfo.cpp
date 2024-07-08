#include "aiminfo.h"

#include "core/angle.h"
#include "core/units.h"
#include "engine/items_tr1.h"
#include "engine/objects/objectstate.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/weapontype.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/actions.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"
#include "weapon.h"

#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <cstdint>
#include <functional>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

namespace engine::objects
{
void AimInfo::serialize(const serialization::Serializer<world::World>& ser) const
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto ptr = reinterpret_cast<const int16_t*>(weaponAnimData);
  ser(S_NV_VECTOR_ELEMENT("weaponAnimData", std::cref(ser.context->getWorldGeometry().getPoseFrames()), std::cref(ptr)),
      S_NV("frame", frame),
      S_NV("aiming", aiming),
      S_NV("aimRotation", aimRotation),
      S_NV("flashTimeout", flashTimeout));
}

void AimInfo::deserialize(const serialization::Deserializer<world::World>& ser)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto ptr = reinterpret_cast<const int16_t*>(weaponAnimData);
  ser(S_NV_VECTOR_ELEMENT("weaponAnimData", std::cref(ser.context->getWorldGeometry().getPoseFrames()), std::ref(ptr)),
      S_NV("frame", frame),
      S_NV("aiming", aiming),
      S_NV("aimRotation", aimRotation),
      S_NV("flashTimeout", flashTimeout));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  weaponAnimData = reinterpret_cast<const loader::file::AnimFrame*>(ptr);
}

void AimInfo::updateAnimTwoWeapons(LaraObject& lara, const Weapon& weapon)
{
  if(!aiming
     && (lara.aimAt != nullptr || !lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)))
  {
    if(frame >= TwoWeaponsRecoilAnimStart)
    {
      frame += 1_frame;
      if(frame == weapon.recoilDuration + TwoWeaponsRecoilAnimStart)
      {
        frame = TwoWeaponsAiming;
      }
    }
    else if(frame > TwoWeaponsIdle && frame <= TwoWeaponsAiming)
    {
      frame -= 1_frame;
    }
  }
  else if(frame >= TwoWeaponsIdle && frame < TwoWeaponsAiming)
  {
    frame += 1_frame;
  }
  else if(frame == TwoWeaponsAiming && lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
  {
    core::TRRotationXY aimAngle;
    aimAngle.X = aimRotation.X;
    aimAngle.Y = lara.m_state.rotation.Y + aimRotation.Y;
    if(lara.tryShootShot(weapon.type, lara.aimAt, lara, aimAngle))
    {
      flashTimeout = weapon.flashTime;
      lara.playSoundEffect(weapon.shotSound);
    }
    frame = TwoWeaponsRecoilAnimStart;
  }
  else if(frame >= TwoWeaponsRecoilAnimStart)
  {
    frame += 1_frame;
    if(frame == weapon.recoilDuration + TwoWeaponsRecoilAnimStart)
    {
      frame = TwoWeaponsAiming;
    }
  }
}

void AimInfo::updateAnimShotgunAiming(LaraObject& lara)
{
  if(frame >= ShotgunIdle && frame <= ShotgunIdleToAimAnimEnd)
  {
    if(frame == ShotgunIdleToAimAnimEnd)
    {
      frame = ShotgunReadyToShoot;
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame == ShotgunReadyToShoot)
  {
    if(lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
    {
      lara.tryShootShotgun();
      frame += 1_frame;
    }
  }
  else if(frame >= ShotgunAfterShotAnimStart && frame <= ShotgunAfterShotAnimEnd)
  {
    if(frame == ShotgunAfterShotAnimEnd)
    {
      frame = ShotgunReadyToShoot;
    }
    else if(frame == ShotgunReload)
    {
      frame += 1_frame;
      lara.playSoundEffect(TR1SoundEffect::LaraHolsterWeapons);
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame >= ShotgunAimToIdleAnimStart && frame <= ShotgunAimToIdleAnimEnd)
  {
    if(frame == ShotgunAimToIdleAnimEnd)
    {
      frame = ShotgunIdle;
    }
    else
    {
      frame += 1_frame;
    }
  }
}

void AimInfo::updateAnimShotgun(LaraObject& lara)
{
  if(aiming)
  {
    updateAnimShotgunAiming(lara);

    return;
  }

  if(frame == ShotgunIdle && lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
  {
    frame += 1_frame;
    return;
  }

  if(frame > ShotgunIdle && frame <= ShotgunIdleToAimAnimEnd)
  {
    if(frame == ShotgunIdleToAimAnimEnd)
    {
      if(lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
      {
        frame = ShotgunReadyToShoot;
      }
      else
      {
        frame = ShotgunAimToIdleAnimStart;
      }
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame == ShotgunReadyToShoot)
  {
    if(lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
    {
      lara.tryShootShotgun();
      frame += 1_frame;
    }
    else
    {
      frame = ShotgunAimToIdleAnimStart;
    }
  }
  else if(frame >= ShotgunAfterShotAnimStart && frame <= ShotgunAfterShotAnimEnd)
  {
    if(frame == ShotgunAfterShotIdle)
    {
      frame = ShotgunIdle;
    }
    else if(frame == ShotgunAfterShotAnimEnd)
    {
      frame = ShotgunAimToIdleAnimStart;
    }
    else if(frame == ShotgunReload)
    {
      lara.playSoundEffect(TR1SoundEffect::LaraHolsterWeapons);
      frame += 1_frame;
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame >= ShotgunAimToIdleAnimStart && frame <= ShotgunAimToIdleAnimEnd)
  {
    if(frame == ShotgunAimToIdleAnimEnd)
    {
      frame = ShotgunIdle;
    }
    else
    {
      frame += 1_frame;
    }
  }
}

void AimInfo::holsterTwoWeapons(LaraObject& lara, WeaponType weaponType)
{
  if(frame >= TwoWeaponsRecoilAnimStart)
  {
    frame = TwoWeaponsAiming;
  }
  else if(frame > TwoWeaponsIdle && frame <= TwoWeaponsAiming)
  {
    aimRotation.X -= aimRotation.X / frame * 1_frame;
    aimRotation.Y -= aimRotation.Y / frame * 1_frame;
    frame -= 1_frame;
  }
  else if(frame == TwoWeaponsIdle)
  {
    aimRotation.X = 0_deg;
    aimRotation.Y = 0_deg;
    frame = DrawTwoWeaponsAnimEnd;
  }
  else if(frame > DrawTwoWeaponsAnimStart && frame <= DrawTwoWeaponsAnimEnd)
  {
    frame -= 1_frame;
    if(frame == TwoWeaponsTouchingHolsters)
    {
      overrideHolsterTwoWeaponsMeshes(lara, weaponType);
      lara.playSoundEffect(TR1SoundEffect::LaraHolster);
    }
  }
}

void AimInfo::holsterShotgun(LaraObject& lara)
{
  if(frame == ShotgunIdle)
  {
    frame = HolsterShotgunAnimStart;
  }
  else if(frame >= ShotgunIdle && frame <= ShotgunIdleToAimAnimEnd)
  {
    if(frame == ShotgunIdleToAimAnimEnd)
    {
      frame = ShotgunAimToIdleAnimStart;
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame == ShotgunReadyToShoot)
  {
    frame = ShotgunAimToIdleAnimStart;
  }
  else if(frame >= ShotgunReadyToShoot && frame <= ShotgunAfterShotAnimEnd)
  {
    if(frame == ShotgunAfterShotIdle)
    {
      frame = ShotgunIdle;
    }
    else if(frame == ShotgunAfterShotAnimEnd)
    {
      frame = ShotgunAimToIdleAnimStart;
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame >= ShotgunAimToIdleAnimStart && frame <= ShotgunAimToIdleAnimEnd)
  {
    if(frame == ShotgunAimToIdleAnimEnd)
    {
      frame = HolsterShotgunAnimStart;
    }
    else
    {
      frame += 1_frame;
    }
  }
  else if(frame >= HolsterShotgunAnimStart && frame <= HolsterShotgunAnimEnd)
  {
    if(frame == ShotgunPutHolster)
    {
      lara.overrideLaraMeshesHolsterShotgun();
      frame += 1_frame;
    }
    else if(frame == HolsterShotgunAnimEnd)
    {
      frame = ShotgunIdle;
      aiming = false;
      lara.setHandStatus(HandStatus::None);
      lara.aimAt = nullptr;
    }
    else
    {
      frame += 1_frame;
    }
  }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void AimInfo::overrideHolsterTwoWeaponsMeshes(LaraObject& lara, WeaponType weaponType)
{
  TR1ItemId srcId;
  switch(weaponType)
  {
  case WeaponType::Pistols:
    srcId = TR1ItemId::LaraPistolsAnim;
    break;
  case WeaponType::Magnums:
    srcId = TR1ItemId::LaraMagnumsAnim;
    break;
  case WeaponType::Uzis:
    srcId = TR1ItemId::LaraUzisAnim;
    break;
  default:
    BOOST_THROW_EXCEPTION(std::domain_error("weaponType"));
  }

  const auto& src = *gsl::not_null{lara.getWorld().getWorldGeometry().findAnimatedModelForType(srcId).get()};
  auto& laraSkeleton = *lara.getSkeleton();
  BOOST_ASSERT(src.bones.size() == laraSkeleton.getBoneCount());
  const auto& normalLara
    = *gsl::not_null{lara.getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::Lara).get()};
  BOOST_ASSERT(normalLara.bones.size() == laraSkeleton.getBoneCount());
  laraSkeleton.setMesh(handBoneId, gsl::at(normalLara.bones, handBoneId).mesh);
  laraSkeleton.setMesh(thighBoneId, gsl::at(src.bones, thighBoneId).mesh);
  laraSkeleton.rebuildMesh();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void AimInfo::overrideDrawTwoWeaponsMeshes(LaraObject& lara, WeaponType weaponType)
{
  TR1ItemId id;
  switch(weaponType)
  {
  case WeaponType::Pistols:
    id = TR1ItemId::LaraPistolsAnim;
    break;
  case WeaponType::Magnums:
    id = TR1ItemId::LaraMagnumsAnim;
    break;
  case WeaponType::Uzis:
    id = TR1ItemId::LaraUzisAnim;
    break;
  default:
    BOOST_THROW_EXCEPTION(std::domain_error("weaponType"));
  }

  const auto& src = lara.getWorld().getWorldGeometry().findAnimatedModelForType(id);
  gsl_Assert(src != nullptr);
  auto& laraSkeleton = *lara.getSkeleton();
  BOOST_ASSERT(src->bones.size() == laraSkeleton.getBoneCount());
  const auto& normalLara = *lara.getWorld().getWorldGeometry().findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(normalLara.bones.size() == laraSkeleton.getBoneCount());
  laraSkeleton.setMesh(handBoneId, gsl::at(src->bones, handBoneId).mesh);
  laraSkeleton.setMesh(thighBoneId, gsl::at(normalLara.bones, thighBoneId).mesh);
}

void AimInfo::updateAimAngles(const Weapon& weapon, const core::TRRotationXY& weaponTargetVector) noexcept
{
  core::TRRotationXY targetRot{};
  if(aiming)
  {
    targetRot = weaponTargetVector;
  }

  if(aimRotation.X >= targetRot.X - weapon.aimSpeed * 1_frame
     && aimRotation.X <= targetRot.X + weapon.aimSpeed * 1_frame)
  {
    aimRotation.X = targetRot.X;
  }
  else if(aimRotation.X >= targetRot.X)
  {
    aimRotation.X -= weapon.aimSpeed * 1_frame;
  }
  else
  {
    aimRotation.X += weapon.aimSpeed * 1_frame;
  }

  if(aimRotation.Y >= targetRot.Y - weapon.aimSpeed * 1_frame
     && aimRotation.Y <= weapon.aimSpeed * 1_frame + targetRot.Y)
  {
    aimRotation.Y = targetRot.Y;
  }
  else if(aimRotation.Y >= targetRot.Y)
  {
    aimRotation.Y -= weapon.aimSpeed * 1_frame;
  }
  else
  {
    aimRotation.Y += weapon.aimSpeed * 1_frame;
  }
}
} // namespace engine::objects
