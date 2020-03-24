#pragma once

#include "engine/ai/ai.h"
#include "engine/cameracontroller.h"
#include "engine/collisioninfo.h"
#include "engine/engine.h"
#include "loader/file/animationid.h"
#include "loader/file/larastateid.h"
#include "modelobject.h"

namespace engine
{
struct CollisionInfo;

namespace objects
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
  Unholster,
  Holster,
  Combat
};

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

public:
  LaraObject(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
      , m_underwaterRoute{*engine}
  {
    initGunflares();
  }

  LaraObject(const gsl::not_null<Engine*>& engine,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

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
    m_state.position.position = m_state.position.position + collisionInfo.shift;
    collisionInfo.shift = {0_len, 0_len, 0_len};
  }

private:
  void handleLaraStateOnLand();

  void handleLaraStateDiving();

  void handleLaraStateSwimming();

  void testInteractions(CollisionInfo& collisionInfo);

  //! @brief If "none", we are not allowed to dive until the "Dive" action key is released
  //! @remarks This happens e.g. just after dive-to-swim transition, when players still
  //!          keep the "Dive Forward" action key pressed; in this case, you usually won't go
  //!          diving immediately again.
  //! @fixme Move this to the engine
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

  void addHeadRotationXY(const core::Angle& x,
                         const core::Angle& minX,
                         const core::Angle& maxX,
                         const core::Angle& y,
                         const core::Angle& minY,
                         const core::Angle& maxY)
  {
    m_headRotation.X = util::clamp(m_headRotation.X + x, minX, maxX);
    m_headRotation.Y = util::clamp(m_headRotation.Y + y, minY, maxY);
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
    m_headRotation = {0_deg, 0_deg, 0_deg};
    m_torsoRotation = {0_deg, 0_deg, 0_deg};
  }

  core::TRRotation m_headRotation;
  core::TRRotation m_torsoRotation;

#ifndef NDEBUG
  CollisionInfo lastUsedCollisionInfo;
#endif

  core::Length m_underwaterCurrentStrength = 0_len;
  ai::PathFinder m_underwaterRoute;

  void handleUnderwaterCurrent(CollisionInfo& collisionInfo);

  std::optional<core::Axis> hit_direction;
  core::Frame hit_frame = 0_frame;
  core::Frame explosionStumblingDuration = 0_frame;
  const core::TRVec* forceSourcePosition = nullptr;

  void updateExplosionStumbling()
  {
    const auto rot = angleFromAtan(forceSourcePosition->X - m_state.position.position.X,
                                   forceSourcePosition->Z - m_state.position.position.Z)
                     - 180_deg;
    hit_direction = axisFromAngle(m_state.rotation.Y - rot, 45_deg);
    Expects(hit_direction.has_value());
    if(hit_frame == 0_frame)
    {
      playSoundEffect(TR1SoundId::LaraOof);
    }

    hit_frame += 1_frame;
    if(hit_frame > 34_frame)
    {
      hit_frame = 34_frame;
    }
    explosionStumblingDuration -= 1_frame;
  }

  struct AimInfo
  {
    const loader::file::AnimFrame* weaponAnimData = nullptr;
    core::Frame frame = 0_frame;
    bool aiming = false;
    core::TRRotationXY aimRotation{};
    core::Frame flashTimeout = 0_frame;

    void serialize(const serialization::Serializer& ser);
  };

  enum class WeaponId
  {
    None,
    Pistols,
    AutoPistols,
    Uzi,
    Shotgun
  };

  struct Ammo
  {
    int ammo = 0;
    int hits = 0;
    int misses = 0;

    void serialize(const serialization::Serializer& ser);
  };

  AimInfo leftArm;
  AimInfo rightArm;

  WeaponId gunType = WeaponId::None;
  WeaponId requestedGunType = WeaponId::None;

  Ammo pistolsAmmo;
  Ammo revolverAmmo;
  Ammo uziAmmo;
  Ammo shotgunAmmo;

  std::shared_ptr<ModelObject> target{nullptr};

  struct Range
  {
    core::Angle min = 0_deg;
    core::Angle max = 0_deg;

    void serialize(const serialization::Serializer& ser);
  };

  struct RangeXY
  {
    Range x{};
    Range y{};

    void serialize(const serialization::Serializer& ser);
  };

  struct Weapon
  {
    RangeXY lockAngles{};
    RangeXY leftAngles{};
    RangeXY rightAngles{};
    core::Angle aimSpeed = 0_deg;
    core::Angle shotAccuracy = 0_deg;
    core::Length gunHeight = 0_len;
    core::Health damage = 0_hp;
    core::Length targetDist = 0_len;
    core::Frame recoilFrame = 0_frame;
    core::Frame flashTime = 0_frame;
    TR1SoundId shotSound = TR1SoundId::LaraFootstep;

    void serialize(const serialization::Serializer& ser);

    static Weapon create(const serialization::Serializer& ser)
    {
      Weapon tmp;
      tmp.serialize(ser);
      return tmp;
    }
  };

  std::unordered_map<WeaponId, Weapon> weapons;
  core::TRRotationXY m_weaponTargetVector;
  gsl::not_null<std::shared_ptr<render::scene::Node>> m_gunFlareLeft{
    std::make_shared<render::scene::Node>("gun flare left")};
  gsl::not_null<std::shared_ptr<render::scene::Node>> m_gunFlareRight{
    std::make_shared<render::scene::Node>("gun flare right")};

  void updateLarasWeaponsStatus();

  void updateShotgun();

  void updateGuns(WeaponId weaponId);

  void updateAimingState(const Weapon& weapon);

  void unholster();

  static core::RoomBoundPosition getUpperThirdBBoxCtr(const ModelObject& object);

  void unholsterGuns(WeaponId weaponId);

  void findTarget(const Weapon& weapon);

  void initAimInfoPistol();

  void initAimInfoShotgun();

  void overrideLaraMeshesUnholsterGuns(WeaponId weaponId);

  void overrideLaraMeshesUnholsterShotgun();

  void unholsterShotgun();

  void updateAimAngles(const Weapon& weapon, AimInfo& aimInfo) const;

  void updateAnimShotgun();

  void tryShootShotgun();

  void holsterShotgun();

  void holsterGuns(WeaponId weaponId);

  void updateAnimNotShotgun(WeaponId weaponId);

  bool fireWeapon(WeaponId weaponId,
                  const std::shared_ptr<ModelObject>& targetObject,
                  const ModelObject& gunHolder,
                  const core::TRRotationXY& aimAngle);

  void hitTarget(ModelObject& object, const core::TRVec& hitPos, const core::Health& damage);

  void renderGunFlare(WeaponId weaponId,
                      glm::mat4 m,
                      const gsl::not_null<std::shared_ptr<render::scene::Node>>& flareNode,
                      bool visible) const;

  void drawRoutine();

  void drawRoutineInterpolated(const SkeletalModelNode::InterpolationInfo& interpolationInfo);

  void alignForInteraction(const core::TRVec& offset, const ObjectState& objectState)
  {
    const auto v = objectState.rotation.toMatrix() * glm::vec4{offset.toRenderSystem(), 1.0f};
    const auto p = core::TRVec{glm::vec3{v}};
    m_state.position.position = objectState.position.position + p;
  }

  void burnIfAlive();

  void serialize(const serialization::Serializer& ser) override;

private:
  void initGunflares();
};
} // namespace objects
} // namespace engine
