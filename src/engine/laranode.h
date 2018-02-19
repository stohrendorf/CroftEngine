#pragma once

#include "loader/animationid.h"
#include "loader/larastateid.h"
#include "collisioninfo.h"
#include "engine/lara/abstractstatehandler.h"
#include "engine/items/itemnode.h"
#include "cameracontroller.h"
#include "ai/ai.h"

namespace engine
{
struct CollisionInfo;

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


class LaraNode final : public items::ModelItemNode
{
    using LaraStateId = loader::LaraStateId;

private:
    //! @brief Additional rotation per TR Engine Frame
    core::Angle m_yRotationSpeed{0};
    int m_fallSpeedOverride = 0;
    core::Angle m_movementAngle{0};
    int m_air{core::LaraAir};
    core::Angle m_currentSlideAngle{0};

    HandStatus m_handStatus = HandStatus::None;
    //! @todo Move this to the Level.
    int m_uvAnimTime{0};

    UnderwaterState m_underwaterState = UnderwaterState::OnLand;

public:
    LaraNode(const gsl::not_null<level::Level*>& level,
             const std::string& name,
             const gsl::not_null<const loader::Room*>& room,
             const loader::Item& item,
             const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, false,
                             SaveHitpoints | SaveFlags | SavePosition | NonLot, animatedModel )
            , m_underwaterRoute{*level}
    {
        setAnimIdGlobal( loader::AnimationId::STAY_IDLE );
        setTargetState( LaraStateId::Stop );
        setMovementAngle( m_state.rotation.Y );

        m_underwaterRoute.step = 20 * loader::SectorSize;
        m_underwaterRoute.drop = -20 * loader::SectorSize;
        m_underwaterRoute.fly = loader::QuarterSectorSize;

        Weapon w;
        weapons[WeaponId::None] = w;

        w.sightAngleRange.y.min = -60_deg;
        w.sightAngleRange.y.max = +60_deg;
        w.sightAngleRange.x.min = -60_deg;
        w.sightAngleRange.x.max = +60_deg;
        w.targetingAngleRangeLeft.y.min = -170_deg;
        w.targetingAngleRangeLeft.y.max = +60_deg;
        w.targetingAngleRangeLeft.x.min = -80_deg;
        w.targetingAngleRangeLeft.x.max = +80_deg;
        w.targetingAngleRangeRight.y.min = -60_deg;
        w.targetingAngleRangeRight.y.max = +170_deg;
        w.targetingAngleRangeRight.x.min = -80_deg;
        w.targetingAngleRangeRight.x.max = +80_deg;
        w.rotationCone = +10_deg;
        w.anglePrecision = +8_deg;
        w.height = 650;
        w.healthDamage = 1;
        w.maxDistance = 8*loader::SectorSize;
        w.field_28 = 9;
        w.shootTimeout = 3;
        w.fireSoundId = 8;
        weapons[WeaponId::Pistols] = w;

        w.sightAngleRange.y.min = -60_deg;
        w.sightAngleRange.y.max = +60_deg;
        w.sightAngleRange.x.min = -60_deg;
        w.sightAngleRange.x.max = +60_deg;
        w.targetingAngleRangeLeft.y.min = -170_deg;
        w.targetingAngleRangeLeft.y.max = +60_deg;
        w.targetingAngleRangeLeft.x.min = -80_deg;
        w.targetingAngleRangeLeft.x.max = +80_deg;
        w.targetingAngleRangeRight.y.min = -60_deg;
        w.targetingAngleRangeRight.y.max = +170_deg;
        w.targetingAngleRangeRight.x.min = -80_deg;
        w.targetingAngleRangeRight.x.max = +80_deg;
        w.rotationCone = +10_deg;
        w.anglePrecision = +8_deg;
        w.height = 650;
        w.healthDamage = 2;
        w.maxDistance = 8*loader::SectorSize;
        w.field_28 = 9;
        w.shootTimeout = 3;
        w.fireSoundId = 44;
        weapons[WeaponId::AutoPistols] = w;

        w.sightAngleRange.y.min = -60_deg;
        w.sightAngleRange.y.max = +60_deg;
        w.sightAngleRange.x.min = -60_deg;
        w.sightAngleRange.x.max = +60_deg;
        w.targetingAngleRangeLeft.y.min = -170_deg;
        w.targetingAngleRangeLeft.y.max = +60_deg;
        w.targetingAngleRangeLeft.x.min = -80_deg;
        w.targetingAngleRangeLeft.x.max = +80_deg;
        w.targetingAngleRangeRight.y.min = -60_deg;
        w.targetingAngleRangeRight.y.max = +170_deg;
        w.targetingAngleRangeRight.x.min = -80_deg;
        w.targetingAngleRangeRight.x.max = +80_deg;
        w.rotationCone = +10_deg;
        w.anglePrecision = +8_deg;
        w.height = 650;
        w.healthDamage = 1;
        w.maxDistance = 8*loader::SectorSize;
        w.field_28 = 3;
        w.shootTimeout = 2;
        w.fireSoundId = 43;
        weapons[WeaponId::Uzi] = w;

        w.sightAngleRange.y.min = -60_deg;
        w.sightAngleRange.y.max = +60_deg;
        w.sightAngleRange.x.min = -55_deg;
        w.sightAngleRange.x.max = +55_deg;
        w.targetingAngleRangeLeft.y.min = -80_deg;
        w.targetingAngleRangeLeft.y.max = +80_deg;
        w.targetingAngleRangeLeft.x.min = -65_deg;
        w.targetingAngleRangeLeft.x.max = +65_deg;
        w.targetingAngleRangeRight.y.min = -80_deg;
        w.targetingAngleRangeRight.y.max = +80_deg;
        w.targetingAngleRangeRight.x.min = -65_deg;
        w.targetingAngleRangeRight.x.max = +65_deg;
        w.rotationCone = +10_deg;
        w.anglePrecision = 0_deg;
        w.height = 500;
        w.healthDamage = 4;
        w.maxDistance = 8*loader::SectorSize;
        w.field_28 = 9;
        w.shootTimeout = 3;
        w.fireSoundId = 45;
        weapons[WeaponId::Shotgun] = w;

        m_state.health = core::LaraHealth;
    }

    ~LaraNode() override;

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

    int getAir() const
    {
        return m_air;
    }

    void updateImpl();

    void update() override;

    void applyShift(const CollisionInfo& collisionInfo)
    {
        m_state.position.position = m_state.position.position + collisionInfo.shift;
        collisionInfo.shift = {0, 0, 0};
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
    int m_swimToDiveKeypressDuration = 0;
    uint16_t m_secretsFoundBitmask = 0;

public:
    void setAir(int a) noexcept
    {
        m_air = a;
    }

    void setMovementAngle(core::Angle angle) noexcept
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

    void setHandStatus(HandStatus status) noexcept
    {
        m_handStatus = status;
    }

    void placeOnFloor(const CollisionInfo& collisionInfo);

    void setYRotationSpeed(core::Angle spd)
    {
        m_yRotationSpeed = spd;
    }

    core::Angle getYRotationSpeed() const
    {
        return m_yRotationSpeed;
    }

    void subYRotationSpeed(core::Angle val, core::Angle limit = -32768_au)
    {
        m_yRotationSpeed = std::max( m_yRotationSpeed - val, limit );
    }

    void addYRotationSpeed(core::Angle val, core::Angle limit = 32767_au)
    {
        m_yRotationSpeed = std::min( m_yRotationSpeed + val, limit );
    }

    void setFallSpeedOverride(int v)
    {
        m_fallSpeedOverride = v;
    }

    core::Angle getCurrentSlideAngle() const noexcept
    {
        return m_currentSlideAngle;
    }

    void setCurrentSlideAngle(core::Angle a) noexcept
    {
        m_currentSlideAngle = a;
    }

    LaraStateId getTargetState() const;

    void setTargetState(loader::LaraStateId st);

    loader::LaraStateId getCurrentAnimState() const;

    void setAnimIdGlobal(loader::AnimationId anim, const boost::optional<uint16_t>& firstFrame = boost::none);

    void updateFloorHeight(int dy);

    void handleCommandSequence(const uint16_t* floorData, bool skipFirstTriggers);

    boost::optional<int> getWaterSurfaceHeight() const;

    void addSwimToDiveKeypressDuration(int n) noexcept
    {
        m_swimToDiveKeypressDuration += n;
    }

    void setSwimToDiveKeypressDuration(int n) noexcept
    {
        m_swimToDiveKeypressDuration = n;
    }

    int getSwimToDiveKeypressDuration() const noexcept
    {
        return m_swimToDiveKeypressDuration;
    }

    void setUnderwaterState(UnderwaterState u) noexcept
    {
        m_underwaterState = u;
    }

    void setCameraCurrentRotation(core::Angle x, core::Angle y);

    void setCameraCurrentRotationX(core::Angle x);

    void setCameraCurrentRotationY(core::Angle y);

    void setCameraTargetDistance(int d);

    void setCameraOldMode(CameraMode k);

    void addHeadRotationXY(const core::Angle& x, const core::Angle& minX, const core::Angle& maxX, const core::Angle& y,
                           const core::Angle& minY, const core::Angle& maxY)
    {
        m_headRotation.X = util::clamp( m_headRotation.X + x, minX, maxX );
        m_headRotation.Y = util::clamp( m_headRotation.Y + y, minY, maxY );
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

    int m_underwaterCurrentStrength = 0;
    ai::LotInfo m_underwaterRoute;

    void handleUnderwaterCurrent(CollisionInfo& collisionInfo);

    boost::optional<core::Axis> hit_direction;
    int hit_frame = 0;
    int explosionStumblingDuration = 0;
    const core::TRCoordinates* forceSourcePosition = nullptr;

    void updateExplosionStumbling()
    {
        const auto rot = core::Angle::fromAtan(
                forceSourcePosition->X - m_state.position.position.X,
                forceSourcePosition->Z - m_state.position.position.Z) - 180_deg;
        hit_direction = core::axisFromAngle(m_state.rotation.Y - rot, 45_deg);
        Expects(hit_direction.is_initialized());
        if ( hit_frame == 0 )
        {
            playSoundEffect(0x1b);
        }
        if ( ++hit_frame > 34 )
        {
            hit_frame = 34;
        }
        --explosionStumblingDuration;
    }

    struct AimInfo
    {
        const loader::AnimFrame *weaponAnimData = nullptr;
        int16_t frame = 0;
        bool aiming = false;
        core::TRRotation aimRotation{};
        int16_t shootTimeout = 0;
    };


    enum class WeaponId {
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
    };

    AimInfo leftArm;
    AimInfo rightArm;

    WeaponId gunType = WeaponId::None;
    WeaponId requestedGunType = WeaponId::Pistols;

    Ammo pistolsAmmo;
    Ammo revolverAmmo;
    Ammo uziAmmo;
    Ammo shotgunAmmo;

    std::shared_ptr<ModelItemNode> target{nullptr};

    struct Range {
        core::Angle min = 0_deg;
        core::Angle max = 0_deg;
    };

    struct RangeXY {
        Range x{};
        Range y{};
    };

    struct Weapon {
        RangeXY sightAngleRange{};
        RangeXY targetingAngleRangeLeft{};
        RangeXY targetingAngleRangeRight{};
        core::Angle rotationCone = 0_deg;
        core::Angle anglePrecision = 0_deg;
        int32_t height = 0;
        int32_t healthDamage = 0;
        int32_t maxDistance = 0;
        int16_t field_28 = 0;
        int16_t shootTimeout = 0;
        int16_t fireSoundId = 0;
    };

    std::unordered_map<WeaponId, Weapon> weapons;
    core::TRRotationXY m_enemyLookRot;

    void updateWeaponState();
    void updateShotgun();
    void updateNotShotgun(WeaponId weaponId);
    void updateAimingState(const Weapon& weapon);
    void unholsterReplaceMeshes();
    core::RoomBoundPosition getUpperThirdBBoxCtr(const ModelItemNode& item);
    void unholsterDoubleWeapon(WeaponId weaponId);
    void findTarget(const Weapon& weapon);
    void initAimInfoPistol();
    void initAimInfoShotgun();
    void overrideLaraMeshesUnholsterBothLegs(WeaponId weaponId);
    void overrideLaraMeshesUnholsterShotgun();
    void unholsterShotgunAnimUpdate();
    void updateAimAngles(Weapon& weapon, AimInfo& aimInfo);
    void updateAnimShotgun();
    void tryShootShotgun();
    void playSingleShotShotgun();
    void playSingleShot(WeaponId weaponId);
    void updateAnimNotShotgun(WeaponId weaponId);
    bool tryShoot(WeaponId weaponId, const std::shared_ptr<engine::items::ModelItemNode>& target, const ModelItemNode& gunHolder, const core::TRRotationXY& aimAngle);
    void playShotMissed(const core::RoomBoundPosition& pos);
    void enemyHit(ModelItemNode& item, const core::TRCoordinates& pos, int healthDamage);

    void drawRoutine();
    void drawRoutineInterpolated(const SkeletalModelNode::InterpolationInfo& interpolationInfo);
};
}
