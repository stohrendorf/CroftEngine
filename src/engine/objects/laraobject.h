#pragma once

#include "core/angle.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/ai/pathfinder.h"
#include "engine/cameracontroller.h"
#include "engine/collisioninfo.h"
#include "engine/location.h"
#include "engine/weapontype.h"
#include "loader/file/animationid.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "render/scene/node.h"
#include "serialization/serialization_fwd.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <optional>
#include <string>

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace engine
{
struct InterpolationInfo;
}

namespace loader::file
{
struct AnimFrame;
struct Item;
} // namespace loader::file

namespace engine::objects
{
enum class UnderwaterState
{
  OnLand,
  Diving,
  Swimming
};

enum class HandStatus
{
  None,
  Grabbing,
  DrawWeapon,
  Holster,
  Combat
};

struct Weapon;

class LaraObject final : public ModelObject
{
  using LaraStateId = loader::file::LaraStateId;
  using AnimationId = loader::file::AnimationId;

private:
  //! @brief Additional rotation per TR Engine Frame
  core::Angle m_yRotationSpeed{0_deg};
  core::Speed m_fallSpeedOverride = 0_spd;
  core::Angle m_movementAngle{0_deg};
  core::Frame m_air{core::LaraAir};
  core::Angle m_currentSlideAngle{0_deg};

  HandStatus m_handStatus = HandStatus::None;

  UnderwaterState m_underwaterState = UnderwaterState::OnLand;

#ifndef NDEBUG
  bool m_cheatDive = false;
#else
  static constexpr bool m_cheatDive = false;
#endif

public:
  LaraObject(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
    initMuzzleFlashes();
  }

  LaraObject(const std::string& name,
             const gsl::not_null<world::World*>& world,
             const gsl::not_null<const world::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  LaraObject(const LaraObject&) = delete;
  LaraObject(LaraObject&&) = delete;
  LaraObject& operator=(const LaraObject&) = delete;
  LaraObject& operator=(LaraObject&&) = delete;

  ~LaraObject() override;

  bool isInWater() const
  {
    return m_underwaterState == UnderwaterState::Swimming || m_underwaterState == UnderwaterState::Diving;
  }

  bool isDiving() const
  {
    return m_underwaterState == UnderwaterState::Diving;
  }

  bool isOnLand() const
  {
    return m_underwaterState == UnderwaterState::OnLand;
  }

  core::Frame getAir() const
  {
    return m_air;
  }

  void updateImpl();

  void update() override;

  void applyShift(const CollisionInfo& collisionInfo)
  {
    m_state.location.position += collisionInfo.shift;
    collisionInfo.shift = {0_len, 0_len, 0_len};
  }

private:
  void handleLaraStateOnLand();
  void handleLaraStateDiving();
  void handleLaraStateSwimming();
  void testInteractions(CollisionInfo& collisionInfo);

  core::Frame m_swimToDiveKeypressDuration = 0_frame;

public:
  void setAir(const core::Frame& a) noexcept
  {
    m_air = a;
  }

  void setMovementAngle(const core::Angle& angle) noexcept
  {
    m_movementAngle = angle;
  }

  core::Angle getMovementAngle() const override
  {
    return m_movementAngle;
  }

  HandStatus getHandStatus() const noexcept
  {
    return m_handStatus;
  }

  void setHandStatus(const HandStatus status) noexcept
  {
    m_handStatus = status;
  }

  void placeOnFloor(const CollisionInfo& collisionInfo);

  void setYRotationSpeed(const core::Angle& spd)
  {
    m_yRotationSpeed = spd;
  }

  core::Angle getYRotationSpeed() const
  {
    return m_yRotationSpeed;
  }

  void subYRotationSpeed(const core::Angle& val, const core::Angle& limit = -32768_au)
  {
    m_yRotationSpeed = std::max(m_yRotationSpeed - val, limit);
  }

  void addYRotationSpeed(const core::Angle& val, const core::Angle& limit = 32767_au)
  {
    m_yRotationSpeed = std::min(m_yRotationSpeed + val, limit);
  }

  void setFallSpeedOverride(const core::Speed& v)
  {
    m_fallSpeedOverride = v;
  }

  core::Angle getCurrentSlideAngle() const noexcept
  {
    return m_currentSlideAngle;
  }

  void setCurrentSlideAngle(const core::Angle& a) noexcept
  {
    m_currentSlideAngle = a;
  }

  loader::file::LaraStateId getGoalAnimState() const
  {
    return static_cast<LaraStateId>(m_state.goal_anim_state.get());
  }

  void setGoalAnimState(LaraStateId st)
  {
    m_state.goal_anim_state = static_cast<uint16_t>(st);
  }

  loader::file::LaraStateId getCurrentAnimState() const
  {
    return static_cast<loader::file::LaraStateId>(m_state.current_anim_state.get());
  }

  void setCurrentAnimState(LaraStateId st)
  {
    m_state.current_anim_state = static_cast<uint16_t>(st);
  }

  void setRequiredAnimState(LaraStateId st)
  {
    m_state.required_anim_state = static_cast<uint16_t>(st);
  }

  void setAnimation(AnimationId anim, const std::optional<core::Frame>& firstFrame = std::nullopt);

  void updateFloorHeight(const core::Length& dy);

  void addSwimToDiveKeypressDuration(const core::Frame& n) noexcept
  {
    m_swimToDiveKeypressDuration += n;
  }

  void setSwimToDiveKeypressDuration(const core::Frame& n) noexcept
  {
    m_swimToDiveKeypressDuration = n;
  }

  core::Frame getSwimToDiveKeypressDuration() const noexcept
  {
    return m_swimToDiveKeypressDuration;
  }

  void setUnderwaterState(const UnderwaterState u) noexcept
  {
    m_underwaterState = u;
  }

  void setCameraRotationAroundLara(const core::Angle& x, const core::Angle& y);

  void setCameraRotationAroundLaraX(const core::Angle& x);

  void setCameraRotationAroundLaraY(const core::Angle& y);

  void setCameraDistance(const core::Length& d);

  void setCameraModifier(CameraModifier k);

  void addHeadRotationX(const core::Angle& x, const core::Angle& minX, const core::Angle& maxX)
  {
    m_headRotation.X = std::clamp(m_headRotation.X + x, minX, maxX);
  }

  void addHeadRotationY(const core::Angle& y, const core::Angle& minY, const core::Angle& maxY)
  {
    m_headRotation.Y = std::clamp(m_headRotation.Y + y, minY, maxY);
  }

  const core::TRRotation& getHeadRotation() const noexcept
  {
    return m_headRotation;
  }

  void setTorsoRotation(const core::TRRotation& r)
  {
    m_torsoRotation = r;
  }

  const core::TRRotation& getTorsoRotation() const noexcept
  {
    return m_torsoRotation;
  }

  void resetHeadTorsoRotation()
  {
    m_headRotation = core::TRRotation{};
    m_torsoRotation = core::TRRotation{};
  }

  core::TRRotation m_headRotation;
  core::TRRotation m_torsoRotation;

  core::Length m_underwaterCurrentStrength = 0_len;
  ai::PathFinder m_underwaterRoute;

  void handleUnderwaterCurrent(CollisionInfo& collisionInfo);

  std::optional<core::Axis> hit_direction;
  core::Frame hit_frame = 0_frame;
  core::Frame explosionStumblingDuration = 0_frame;
  const core::TRVec* forceSourcePosition = nullptr;

  void updateExplosionStumbling();

  struct AimInfo
  {
    const size_t handBoneId;
    const size_t thighBoneId;

    const loader::file::AnimFrame* weaponAnimData = nullptr;
    core::Frame frame = 0_frame;
    bool aiming = false;
    core::TRRotationXY aimRotation{};
    core::Frame flashTimeout = 0_frame;

    void update(LaraObject& lara, const Weapon& weapon);
    void holsterWeapons(LaraObject& lara, WeaponType weaponType);
    void overrideHolsterWeaponsMeshes(LaraObject& lara, WeaponType weaponType);

    void serialize(const serialization::Serializer<world::World>& ser);

    void reset()
    {
      frame = 0_frame;
      aiming = false;
      aimRotation.X = 0_deg;
      aimRotation.Y = 0_deg;
    }
  };

  AimInfo leftArm{13, 1};
  AimInfo rightArm{10, 4};

  std::shared_ptr<ModelObject> aimAt{nullptr};

  core::TRRotationXY m_weaponTargetVector;
  gsl::not_null<std::shared_ptr<render::scene::Node>> m_muzzleFlashLeft{
    std::make_shared<render::scene::Node>("muzzle flash left")};
  gsl::not_null<std::shared_ptr<render::scene::Node>> m_muzzleFlashRight{
    std::make_shared<render::scene::Node>("muzzle flash right")};

  void updateLarasWeaponsStatus();

  void updateShotgun();

  void updateWeapons(WeaponType weaponType);

  void updateAimingState(const Weapon& weapon);

  void initWeaponAnimData();

  static Location getUpperThirdBBoxCtr(const ModelObject& object);

  void drawWeapons(WeaponType weaponType);

  void findTarget(const Weapon& weapon);

  void initAimInfoPistol();

  void initAimInfoShotgun();

  void overrideLaraMeshesDrawWeapons(WeaponType weaponType);

  void overrideLaraMeshesDrawShotgun();

  void drawShotgun();

  void updateAimAngles(const Weapon& weapon, AimInfo& aimInfo) const;

  void updateAnimShotgun();

  void tryShootShotgun();

  void holsterShotgun();

  void holsterWeapons(WeaponType weaponType);

  void updateAnimNotShotgun(WeaponType weaponType);

  bool shootBullet(const WeaponType weaponType,
                   const std::shared_ptr<ModelObject>& targetObject,
                   const ModelObject& weaponHolder,
                   const core::TRRotationXY& aimAngle);

  void hitTarget(ModelObject& object, const core::TRVec& hitPos, const core::Health& damage);

  void renderMuzzleFlash(WeaponType weaponType,
                         glm::mat4 m,
                         const gsl::not_null<std::shared_ptr<render::scene::Node>>& muzzleFlashNode,
                         bool visible) const;

  void drawRoutine();

  void drawRoutineInterpolated(const InterpolationInfo& interpolationInfo);

  void alignForInteraction(const core::TRVec& offset, const ObjectState& objectState)
  {
    const auto v = objectState.rotation.toMatrix() * glm::vec4{offset.toRenderSystem(), 1.0f};
    const auto p = core::TRVec{glm::vec3{v}};
    m_state.location.position = objectState.location.position + p;
    m_state.rotation = objectState.rotation;
  }

  void burnIfAlive();

  void serialize(const serialization::Serializer<world::World>& ser) override;

  bool isDead() const
  {
    return m_state.isDead();
  }

private:
  void initMuzzleFlashes();
};
} // namespace engine::objects
