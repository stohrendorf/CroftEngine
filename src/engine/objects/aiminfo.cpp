#include "aiminfo.h"

#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "hid/inputhandler.h"
#include "laraobject.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector_element.h"
#include "weapon.h"

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

void AimInfo::update(LaraObject& lara, const Weapon& weapon)
{
  if(!aiming
     && (lara.aimAt != nullptr || !lara.getWorld().getPresenter().getInputHandler().hasAction(hid::Action::Action)))
  {
    if(frame >= TwoWeaponsRecoilAnimStart)
    {
      frame = TwoWeaponsAiming;
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
    if(lara.shootBullet(weapon.type, lara.aimAt, lara, aimAngle))
    {
      flashTimeout = weapon.flashTime;
      lara.playSoundEffect(weapon.shotSound);
    }
    frame = TwoWeaponsRecoilAnimStart;
  }
  else if(frame >= TwoWeaponsRecoilAnimStart)
  {
    frame += 1_frame;
    if(frame == weapon.recoilFrame + TwoWeaponsRecoilAnimStart)
    {
      frame = TwoWeaponsAiming;
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
} // namespace engine::objects
