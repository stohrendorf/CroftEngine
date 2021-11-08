#include "aiminfo.h"

#include "engine/items_tr1.h"
#include "engine/objects/objectstate.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
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
#include <exception>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

namespace engine::objects
{
void AimInfo::serialize(const serialization::Serializer<world::World>& ser)
{
  auto ptr = reinterpret_cast<const int16_t*>(weaponAnimData);
  ser(S_NV_VECTOR_ELEMENT("weaponAnimData", ser.context.getPoseFrames(), ptr),
      S_NV("frame", frame),
      S_NV("aiming", aiming),
      S_NV("aimRotation", aimRotation),
      S_NV("flashTimeout", flashTimeout));
  weaponAnimData = reinterpret_cast<const loader::file::AnimFrame*>(ptr);
}

void AimInfo::updateAnimTwoWeapons(LaraObject& lara, const Weapon& weapon)
{
  if(!aiming
     && (lara.aimAt != nullptr || !lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)))
  {
    if(frame >= toAnimUnit(TwoWeaponsRecoilAnimStart))
    {
      frame += 1_rframe;
      if(frame == toAnimUnit(weapon.recoilDuration + TwoWeaponsRecoilAnimStart))
      {
        frame = toAnimUnit(TwoWeaponsAiming);
      }
    }
    else if(frame > toAnimUnit(TwoWeaponsIdle) && frame <= toAnimUnit(TwoWeaponsAiming))
    {
      frame -= 1_rframe;
    }
  }
  else if(frame >= toAnimUnit(TwoWeaponsIdle) && frame < toAnimUnit(TwoWeaponsAiming))
  {
    frame += 1_rframe;
  }
  else if(frame == toAnimUnit(TwoWeaponsAiming)
          && lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
  {
    core::TRRotationXY aimAngle;
    aimAngle.X = aimRotation.X;
    aimAngle.Y = lara.m_state.rotation.Y + aimRotation.Y;
    if(lara.shootBullet(weapon.type, lara.aimAt, lara, aimAngle))
    {
      flashTimeout = toAnimUnit(weapon.flashTime);
      lara.playSoundEffect(weapon.shotSound);
    }
    frame = toAnimUnit(TwoWeaponsRecoilAnimStart);
  }
  else if(frame >= toAnimUnit(TwoWeaponsRecoilAnimStart))
  {
    frame += 1_rframe;
    if(frame == toAnimUnit(weapon.recoilDuration + TwoWeaponsRecoilAnimStart))
    {
      frame = toAnimUnit(TwoWeaponsAiming);
    }
  }
}

void AimInfo::updateAnimShotgun(LaraObject& lara)
{
  if(aiming)
  {
    if(frame >= toAnimUnit(ShotgunIdle) && frame <= toAnimUnit(ShotgunIdleToAimAnimEnd))
    {
      if(frame == toAnimUnit(ShotgunIdleToAimAnimEnd))
      {
        frame = toAnimUnit(ShotgunReadyToShoot);
      }
      else
      {
        frame += 1_rframe;
      }
    }
    else if(frame == toAnimUnit(ShotgunReadyToShoot))
    {
      if(lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
      {
        lara.tryShootShotgun();
        frame += 1_rframe;
      }
    }
    else if(frame >= toAnimUnit(ShotgunAfterShotAnimStart) && frame <= toAnimUnit(ShotgunAfterShotAnimEnd))
    {
      if(frame == toAnimUnit(ShotgunAfterShotAnimEnd))
      {
        frame = toAnimUnit(ShotgunReadyToShoot);
      }
      else if(frame == toAnimUnit(ShotgunReload))
      {
        frame += 1_rframe;
        lara.playSoundEffect(TR1SoundEffect::LaraHolsterWeapons);
      }
      else
      {
        frame += 1_rframe;
      }
    }
    else if(frame >= toAnimUnit(ShotgunAimToIdleAnimStart) && frame <= toAnimUnit(ShotgunAimToIdleAnimEnd))
    {
      if(frame == toAnimUnit(ShotgunAimToIdleAnimEnd))
      {
        frame = toAnimUnit(ShotgunIdle);
      }
      else
      {
        frame += 1_rframe;
      }
    }

    return;
  }

  if(frame == toAnimUnit(ShotgunIdle)
     && lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
  {
    frame += 1_rframe;
    return;
  }

  if(frame > toAnimUnit(ShotgunIdle) && frame <= toAnimUnit(ShotgunIdleToAimAnimEnd))
  {
    if(frame == toAnimUnit(ShotgunIdleToAimAnimEnd))
    {
      if(lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
      {
        frame = toAnimUnit(ShotgunReadyToShoot);
      }
      else
      {
        frame = toAnimUnit(ShotgunAimToIdleAnimStart);
      }
    }
    else
    {
      frame += 1_rframe;
    }
  }
  else if(frame == toAnimUnit(ShotgunReadyToShoot))
  {
    if(lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action))
    {
      lara.tryShootShotgun();
      frame += 1_rframe;
    }
    else
    {
      frame = toAnimUnit(ShotgunAimToIdleAnimStart);
    }
  }
  else if(frame >= toAnimUnit(ShotgunAfterShotAnimStart) && frame <= toAnimUnit(ShotgunAfterShotAnimEnd))
  {
    if(frame == toAnimUnit(ShotgunAfterShotIdle))
    {
      frame = toAnimUnit(ShotgunIdle);
    }
    else if(frame == toAnimUnit(ShotgunAfterShotAnimEnd))
    {
      frame = toAnimUnit(ShotgunAimToIdleAnimStart);
    }
    else if(frame == toAnimUnit(ShotgunReload))
    {
      lara.playSoundEffect(TR1SoundEffect::LaraHolsterWeapons);
      frame += 1_rframe;
    }
    else
    {
      frame += 1_rframe;
    }
  }
  else if(frame >= toAnimUnit(ShotgunAimToIdleAnimStart) && frame <= toAnimUnit(ShotgunAimToIdleAnimEnd))
  {
    if(frame == toAnimUnit(ShotgunAimToIdleAnimEnd))
    {
      frame = toAnimUnit(ShotgunIdle);
    }
    else
    {
      frame += 1_rframe;
    }
  }
  else
  {
    frame += 1_rframe;
  }
}

void AimInfo::holsterTwoWeapons(LaraObject& lara, WeaponType weaponType)
{
  if(frame >= toAnimUnit(TwoWeaponsRecoilAnimStart))
  {
    frame = toAnimUnit(TwoWeaponsAiming);
  }
  else if(frame > toAnimUnit(TwoWeaponsIdle) && frame < toAnimUnit(TwoWeaponsAiming + 1_frame))
  {
    aimRotation.X -= aimRotation.X / frame * 1_rframe;
    aimRotation.Y -= aimRotation.Y / frame * 1_rframe;
    frame -= 1_rframe;
  }
  else if(frame == toAnimUnit(TwoWeaponsIdle))
  {
    aimRotation.X = 0_deg;
    aimRotation.Y = 0_deg;
    frame = toAnimUnit(DrawTwoWeaponsAnimEnd);
  }
  else if(frame > toAnimUnit(DrawTwoWeaponsAnimStart) && frame < toAnimUnit(DrawTwoWeaponsAnimEnd + 1_frame))
  {
    frame -= 1_rframe;
    if(frame == toAnimUnit(TwoWeaponsTouchingHolsters))
    {
      overrideHolsterTwoWeaponsMeshes(lara, weaponType);
      lara.playSoundEffect(TR1SoundEffect::LaraHolster);
    }
  }
}

void AimInfo::holsterShotgun(LaraObject& lara)
{
  if(frame == toAnimUnit(ShotgunIdle))
  {
    frame = toAnimUnit(HolsterShotgunAnimStart);
  }
  else if(frame >= toAnimUnit(ShotgunIdle) && frame < toAnimUnit(ShotgunIdleToAimAnimEnd + 1_frame))
  {
    if(frame == toAnimUnit(ShotgunIdleToAimAnimEnd))
    {
      frame = toAnimUnit(ShotgunAimToIdleAnimStart);
    }
    else
    {
      frame += 1_rframe;
    }
  }
  else if(frame == toAnimUnit(ShotgunReadyToShoot))
  {
    frame = toAnimUnit(ShotgunAimToIdleAnimStart);
  }
  else if(frame >= toAnimUnit(ShotgunReadyToShoot) && frame < toAnimUnit(ShotgunAfterShotAnimEnd + 1_frame))
  {
    if(frame == toAnimUnit(ShotgunAfterShotIdle))
    {
      frame = toAnimUnit(ShotgunIdle);
    }
    else if(frame == toAnimUnit(ShotgunAfterShotAnimEnd))
    {
      frame = toAnimUnit(ShotgunAimToIdleAnimStart);
    }
    else
    {
      frame += 1_rframe;
    }
  }
  else if(frame >= toAnimUnit(ShotgunAimToIdleAnimStart) && frame < toAnimUnit(ShotgunAimToIdleAnimEnd + 1_frame))
  {
    if(frame == toAnimUnit(ShotgunAimToIdleAnimEnd))
    {
      frame = toAnimUnit(HolsterShotgunAnimStart);
    }
    else
    {
      frame += 1_rframe;
    }
  }
  else if(frame >= toAnimUnit(HolsterShotgunAnimStart) && frame < toAnimUnit(HolsterShotgunAnimEnd + 1_frame))
  {
    if(frame == toAnimUnit(ShotgunPutHolster))
    {
      lara.overrideLaraMeshesHolsterShotgun();
      frame += 1_rframe;
    }
    else if(frame == toAnimUnit(HolsterShotgunAnimEnd))
    {
      frame = toAnimUnit(ShotgunIdle);
      aiming = false;
      lara.setHandStatus(HandStatus::None);
      lara.aimAt = nullptr;
    }
    else
    {
      frame += 1_rframe;
    }
  }
}

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

  const auto& src = *lara.getWorld().findAnimatedModelForType(srcId);
  BOOST_ASSERT(src.bones.size() == lara.getSkeleton()->getBoneCount());
  const auto& normalLara = *lara.getWorld().findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(normalLara.bones.size() == lara.getSkeleton()->getBoneCount());
  lara.getSkeleton()->setMeshPart(handBoneId, normalLara.bones[handBoneId].mesh);
  lara.getSkeleton()->setMeshPart(thighBoneId, src.bones[thighBoneId].mesh);
  lara.getSkeleton()->rebuildMesh();
}

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

  const auto& src = lara.getWorld().findAnimatedModelForType(id);
  Expects(src != nullptr);
  BOOST_ASSERT(src->bones.size() == lara.getSkeleton()->getBoneCount());
  const auto& normalLara = *lara.getWorld().findAnimatedModelForType(TR1ItemId::Lara);
  BOOST_ASSERT(normalLara.bones.size() == lara.getSkeleton()->getBoneCount());
  lara.getSkeleton()->setMeshPart(handBoneId, src->bones[handBoneId].mesh);
  lara.getSkeleton()->setMeshPart(thighBoneId, normalLara.bones[thighBoneId].mesh);
}

void AimInfo::updateAimAngles(const Weapon& weapon, const core::TRRotationXY& weaponTargetVector)
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
