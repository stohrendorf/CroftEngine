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
    core::Speed m_fallSpeedOverride = 0_spd;
    core::Angle m_movementAngle{0};
    core::Frame m_air{core::LaraAir};
    core::Angle m_currentSlideAngle{0};

    HandStatus m_handStatus = HandStatus::None;

    UnderwaterState m_underwaterState = UnderwaterState::OnLand;

public:
    LaraNode(const gsl::not_null<level::Level*>& level,
             const gsl::not_null<const loader::Room*>& room,
             const loader::Item& item,
             const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, room, item, false, animatedModel )
            , m_underwaterRoute{*level}
            , m_gunFlareLeft{std::make_shared<gameplay::Node>( "gun flare left" )}
            , m_gunFlareRight{std::make_shared<gameplay::Node>( "gun flare right" )}
    {
        setAnimation( loader::AnimationId::STAY_IDLE );
        setGoalAnimState( LaraStateId::Stop );
        setMovementAngle( m_state.rotation.Y );

        m_underwaterRoute.step = core::SectorSize * 20;
        m_underwaterRoute.drop = -core::SectorSize * 20;
        m_underwaterRoute.fly = core::QuarterSectorSize;

        Weapon w;
        weapons[WeaponId::None] = w;

        w.lockAngles.y.min = -60_deg;
        w.lockAngles.y.max = +60_deg;
        w.lockAngles.x.min = -60_deg;
        w.lockAngles.x.max = +60_deg;
        w.leftAngles.y.min = -170_deg;
        w.leftAngles.y.max = +60_deg;
        w.leftAngles.x.min = -80_deg;
        w.leftAngles.x.max = +80_deg;
        w.rightAngles.y.min = -60_deg;
        w.rightAngles.y.max = +170_deg;
        w.rightAngles.x.min = -80_deg;
        w.rightAngles.x.max = +80_deg;
        w.aimSpeed = +10_deg;
        w.shotAccuracy = +8_deg;
        w.gunHeight = 650_len;
        w.damage = 1_hp;
        w.targetDist = core::SectorSize * 8;
        w.recoilFrame = 9_frame;
        w.flashTime = 3_frame;
        w.sampleNum = TR1SoundId::LaraShootPistols;
        weapons[WeaponId::Pistols] = w;

        w.lockAngles.y.min = -60_deg;
        w.lockAngles.y.max = +60_deg;
        w.lockAngles.x.min = -60_deg;
        w.lockAngles.x.max = +60_deg;
        w.leftAngles.y.min = -170_deg;
        w.leftAngles.y.max = +60_deg;
        w.leftAngles.x.min = -80_deg;
        w.leftAngles.x.max = +80_deg;
        w.rightAngles.y.min = -60_deg;
        w.rightAngles.y.max = +170_deg;
        w.rightAngles.x.min = -80_deg;
        w.rightAngles.x.max = +80_deg;
        w.aimSpeed = +10_deg;
        w.shotAccuracy = +8_deg;
        w.gunHeight = 650_len;
        w.damage = 2_hp;
        w.targetDist = core::SectorSize * 8;
        w.recoilFrame = 9_frame;
        w.flashTime = 3_frame;
        w.sampleNum = TR1SoundId::CowboyShoot;
        weapons[WeaponId::AutoPistols] = w;

        w.lockAngles.y.min = -60_deg;
        w.lockAngles.y.max = +60_deg;
        w.lockAngles.x.min = -60_deg;
        w.lockAngles.x.max = +60_deg;
        w.leftAngles.y.min = -170_deg;
        w.leftAngles.y.max = +60_deg;
        w.leftAngles.x.min = -80_deg;
        w.leftAngles.x.max = +80_deg;
        w.rightAngles.y.min = -60_deg;
        w.rightAngles.y.max = +170_deg;
        w.rightAngles.x.min = -80_deg;
        w.rightAngles.x.max = +80_deg;
        w.aimSpeed = +10_deg;
        w.shotAccuracy = +8_deg;
        w.gunHeight = 650_len;
        w.damage = 1_hp;
        w.targetDist = core::SectorSize * 8;
        w.recoilFrame = 3_frame;
        w.flashTime = 2_frame;
        w.sampleNum = TR1SoundId::LaraShootUzis;
        weapons[WeaponId::Uzi] = w;

        w.lockAngles.y.min = -60_deg;
        w.lockAngles.y.max = +60_deg;
        w.lockAngles.x.min = -55_deg;
        w.lockAngles.x.max = +55_deg;
        w.leftAngles.y.min = -80_deg;
        w.leftAngles.y.max = +80_deg;
        w.leftAngles.x.min = -65_deg;
        w.leftAngles.x.max = +65_deg;
        w.rightAngles.y.min = -80_deg;
        w.rightAngles.y.max = +80_deg;
        w.rightAngles.x.min = -65_deg;
        w.rightAngles.x.max = +65_deg;
        w.aimSpeed = +10_deg;
        w.shotAccuracy = 0_deg;
        w.gunHeight = 500_len;
        w.damage = 4_hp;
        w.targetDist = core::SectorSize * 8;
        w.recoilFrame = 9_frame;
        w.flashTime = 3_frame;
        w.sampleNum = TR1SoundId::LaraShootShotgun;
        weapons[WeaponId::Shotgun] = w;

        m_state.health = core::LaraHealth;
        m_state.collidable = true;
        m_state.is_hit = true;
        m_state.falling = true;

        const auto& gunFlareModel = getLevel().findAnimatedModelForType( TR1ItemId::Gunflare );
        if( gunFlareModel == nullptr )
            return;

        const auto& mdl = gunFlareModel->models[0];

        m_gunFlareLeft->setDrawable( mdl.get() );
        m_gunFlareLeft->setVisible( false );

        m_gunFlareRight->setDrawable( mdl.get() );
        m_gunFlareRight->setVisible( false );
    }

    LaraNode(const LaraNode&) = delete;

    LaraNode(LaraNode&&) = delete;

    LaraNode& operator=(const LaraNode&) = delete;

    LaraNode& operator=(LaraNode&&) = delete;

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
    core::Frame m_swimToDiveKeypressDuration = 0_frame;
    uint16_t m_secretsFoundBitmask = 0;

public:
    void setAir(const core::Frame a) noexcept
    {
        m_air = a;
    }

    void setMovementAngle(const core::Angle angle) noexcept
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

    void setYRotationSpeed(const core::Angle spd)
    {
        m_yRotationSpeed = spd;
    }

    core::Angle getYRotationSpeed() const
    {
        return m_yRotationSpeed;
    }

    void subYRotationSpeed(const core::Angle val, const core::Angle limit = -32768_au)
    {
        m_yRotationSpeed = std::max( m_yRotationSpeed - val, limit );
    }

    void addYRotationSpeed(const core::Angle val, const core::Angle limit = 32767_au)
    {
        m_yRotationSpeed = std::min( m_yRotationSpeed + val, limit );
    }

    void setFallSpeedOverride(const core::Speed v)
    {
        m_fallSpeedOverride = v;
    }

    core::Angle getCurrentSlideAngle() const noexcept
    {
        return m_currentSlideAngle;
    }

    void setCurrentSlideAngle(const core::Angle a) noexcept
    {
        m_currentSlideAngle = a;
    }

    loader::LaraStateId getGoalAnimState() const
    {
        return static_cast<LaraStateId>(m_state.goal_anim_state.get());
    }

    void setGoalAnimState(LaraStateId st)
    {
        m_state.goal_anim_state = static_cast<uint16_t>(st);
    }

    loader::LaraStateId getCurrentAnimState() const
    {
        return static_cast<loader::LaraStateId>(m_state.current_anim_state.get());
    }

    void setCurrentAnimState(LaraStateId st)
    {
        m_state.current_anim_state = static_cast<uint16_t>(st);
    }

    void setRequiredAnimState(LaraStateId st)
    {
        m_state.required_anim_state = static_cast<uint16_t>(st);
    }

    void setAnimation(loader::AnimationId anim, const boost::optional<core::Frame>& firstFrame = boost::none);

    void updateFloorHeight(core::Length dy);

    void handleCommandSequence(const engine::floordata::FloorDataValue* floorData, bool fromHeavy);

    void addSwimToDiveKeypressDuration(const core::Frame n) noexcept
    {
        m_swimToDiveKeypressDuration += n;
    }

    void setSwimToDiveKeypressDuration(const core::Frame n) noexcept
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

    void setCameraRotationAroundCenter(const core::Angle x, const core::Angle y);

    void setCameraRotationAroundCenterX(const core::Angle x);

    void setCameraRotationAroundCenterY(const core::Angle y);

    void setCameraEyeCenterDistance(core::Length d);

    void setCameraModifier(const CameraModifier k);

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

    core::Length m_underwaterCurrentStrength = 0_len;
    ai::LotInfo m_underwaterRoute;

    void handleUnderwaterCurrent(CollisionInfo& collisionInfo);

    boost::optional<core::Axis> hit_direction;
    core::Frame hit_frame = 0_frame;
    core::Frame explosionStumblingDuration = 0_frame;
    const core::TRVec* forceSourcePosition = nullptr;

    void updateExplosionStumbling()
    {
        const auto rot = core::Angle::fromAtan(
                forceSourcePosition->X - m_state.position.position.X,
                forceSourcePosition->Z - m_state.position.position.Z ) - 180_deg;
        hit_direction = axisFromAngle( m_state.rotation.Y - rot, 45_deg );
        Expects( hit_direction.is_initialized() );
        if( hit_frame == 0_frame )
        {
            playSoundEffect( TR1SoundId::LaraOof );
        }

        hit_frame += 1_frame;
        if( hit_frame > 34_frame )
        {
            hit_frame = 34_frame;
        }
        explosionStumblingDuration -= 1_frame;
    }


    struct AimInfo
    {
        const loader::AnimFrame* weaponAnimData = nullptr;
        core::Frame frame = 0_frame;
        bool aiming = false;
        core::TRRotationXY aimRotation{};
        core::Frame flashTimeout = 0_frame;

        YAML::Node save(const level::Level& lvl) const;

        void load(const YAML::Node& n, const level::Level& lvl);
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

        YAML::Node save() const
        {
            YAML::Node n;
            n.SetStyle( YAML::EmitterStyle::Flow );
            n["ammo"] = ammo;
            n["hits"] = hits;
            n["misses"] = misses;
            return n;
        }

        void load(const YAML::Node& n)
        {
            if( !n["ammo"].IsScalar() )
                BOOST_THROW_EXCEPTION( std::domain_error( "Ammo::ammo is not a scalar value" ) );
            if( !n["hits"].IsScalar() )
                BOOST_THROW_EXCEPTION( std::domain_error( "Ammo::hits is not a scalar value" ) );
            if( !n["misses"].IsScalar() )
                BOOST_THROW_EXCEPTION( std::domain_error( "Ammo::misses is not a scalar value" ) );

            ammo = n["ammo"].as<int>();
            hits = n["hits"].as<int>();
            misses = n["misses"].as<int>();
        }
    };


    AimInfo leftArm;
    AimInfo rightArm;

    WeaponId gunType = WeaponId::None;
    WeaponId requestedGunType = WeaponId::None;

    Ammo pistolsAmmo;
    Ammo revolverAmmo;
    Ammo uziAmmo;
    Ammo shotgunAmmo;

    std::shared_ptr<ModelItemNode> target{nullptr};

    struct Range
    {
        core::Angle min = 0_deg;
        core::Angle max = 0_deg;
    };

    struct RangeXY
    {
        Range x{};
        Range y{};
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
        TR1SoundId sampleNum = TR1SoundId::LaraFootstep;
    };

    std::unordered_map<WeaponId, Weapon> weapons;
    core::TRRotationXY m_weaponTargetVector;
    gsl::not_null<std::shared_ptr<gameplay::Node>> m_gunFlareLeft;
    gsl::not_null<std::shared_ptr<gameplay::Node>> m_gunFlareRight;

    void updateLarasWeaponsStatus();

    void updateShotgun();

    void updateGuns(WeaponId weaponId);

    void updateAimingState(const Weapon& weapon);

    void unholster();

    static core::RoomBoundPosition getUpperThirdBBoxCtr(const ModelItemNode& item);

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
                    const std::shared_ptr<ModelItemNode>& target,
                    const ModelItemNode& gunHolder,
                    const core::TRRotationXY& aimAngle);

    void hitTarget(ModelItemNode& item, const core::TRVec& hitPos, core::Health damage);

    void renderGunFlare(WeaponId weaponId, glm::mat4 m, const gsl::not_null<std::shared_ptr<gameplay::Node>>& flareNode,
                        bool visible) const;

    void drawRoutine();

    void drawRoutineInterpolated(const SkeletalModelNode::InterpolationInfo& interpolationInfo);

    void alignForInteraction(const core::TRVec& offset, const items::ItemState& item)
    {
        const auto v = item.rotation.toMatrix() * glm::vec4{offset.toRenderSystem(), 1.0f};
        const auto p = core::TRVec{glm::vec3{v}};
        m_state.position.position = item.position.position + p;
    }

    YAML::Node save() const override;

    void load(const YAML::Node& n) override;
};
}
