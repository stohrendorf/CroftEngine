#pragma once

#include "audio/sourcehandle.h"
#include "core/boundingbox.h"
#include "core/units.h"
#include "engine/floordata/floordata.h"
#include "engine/skeletalmodelnode.h"
#include "engine/items_tr1.h"
#include "engine/collisioninfo.h"
#include "engine/lighting.h"
#include "engine/sounds_tr1.h"

#include <set>

#include <sol.hpp>

namespace loader
{
namespace file
{
struct Item;
}
}

namespace engine
{
class LaraNode;


class Particle;


class Engine;

namespace ai
{
struct CreatureInfo;
}

struct CollisionInfo;

namespace items
{

struct InteractionLimits
{
    core::BoundingBox distance;
    core::TRRotation minAngle;
    core::TRRotation maxAngle;

    InteractionLimits(const core::BoundingBox& bbox, const core::TRRotation& min, const core::TRRotation& max)
            : distance{bbox}
            , minAngle{min}
            , maxAngle{max}
    {
        distance.makeValid();
    }

    bool canInteract(const ItemState& item, const ItemState& lara) const;
};


enum class TriggerState
{
    Inactive,
    Active,
    Deactivated,
    Invisible
};


struct ItemState final : public audio::Emitter
{
    explicit ItemState(const gsl::not_null<audio::SoundEngine*>& engine,
                       const gsl::not_null<const loader::file::Room*>& room,
                       const core::TypeId type)
            : Emitter{engine}
            , type{type}
            , position{room}
    {}

    ItemState(const ItemState&) = default;

    ItemState(ItemState&&) = default;

    ItemState& operator=(const ItemState&) = default;

    ItemState& operator=(ItemState&&) = default;

    ~ItemState() override;

    glm::vec3 getPosition() const final;

    core::TypeId type;
    core::RoomBoundPosition position;
    core::TRRotation rotation;
    core::Speed speed = 0_spd;
    core::Speed fallspeed = 0_spd;
    core::AnimStateId current_anim_state = 0_as;
    core::AnimStateId goal_anim_state = 0_as;
    core::AnimStateId required_anim_state = 0_as;
    const loader::file::Animation* anim = nullptr;
    core::Frame frame_number = 0_frame;
    core::Health health = 0_hp;
    TriggerState triggerState = TriggerState::Inactive;
    core::Frame timer = 0_frame;
    floordata::ActivationState activationState;
    core::Length floor = 0_len;
    std::bitset<32> touch_bits;
    const loader::file::Box* box = nullptr;
    int16_t shade = -1;
    std::bitset<32> mesh_bits;

    bool falling = false;
    bool is_hit = false;
    bool collidable = true;
    bool already_looked_at = false;
    bool dynamic_light = false;

    std::shared_ptr<ai::CreatureInfo> creatureInfo;

    YAML::Node save(const Engine& engine) const;

    void load(const YAML::Node& n, const Engine& engine);

    bool updateActivationTimeout()
    {
        if( !activationState.isFullyActivated() )
        {
            return activationState.isInverted();
        }

        if( timer == 0_frame )
            return !activationState.isInverted();

        if( timer < 0_frame )
            return activationState.isInverted();

        timer -= 1_frame;
        if( timer == 0_frame )
            timer = -1_frame;

        return !activationState.isInverted();
    }

    bool stalkBox(const Engine& engine, const loader::file::Box& box) const;

    bool
    isInsideZoneButNotInBox(const Engine& engine, int16_t zoneId, const loader::file::Box& box) const;

    bool inSameQuadrantAsBoxRelativeToLara(const Engine& engine, const loader::file::Box& box) const;

    void initCreatureInfo(const Engine& engine);

    void collectZoneBoxes(const Engine& engine);

    const loader::file::Sector* getCurrentSector() const
    {
        return position.room->getSectorByAbsolutePosition( position.position );
    }
};


class ItemNode
{
    const gsl::not_null<Engine*> m_engine;

public:
    ItemState m_state;

    bool m_isActive = false;

    const bool m_hasUpdateFunction;

    Lighting m_lighting;

    enum class AnimCommandOpcode
            : uint16_t
    {
        SetPosition = 1,
        StartFalling = 2,
        EmptyHands = 3,
        Kill = 4,
        PlaySound = 5,
        PlayEffect = 6,
        Interact = 7
    };

    ItemNode(const gsl::not_null<Engine*>& engine,
             const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item,
             bool hasUpdateFunction);

    ItemNode(const ItemNode&) = delete;

    ItemNode(ItemNode&&) = delete;

    ItemNode& operator=(const ItemNode&) = delete;

    ItemNode& operator=(ItemNode&&) = delete;

    virtual ~ItemNode() = default;

    virtual void update() = 0;

    virtual std::shared_ptr<gameplay::Node> getNode() const = 0;

    void setCurrentRoom(const gsl::not_null<const loader::file::Room*>& newRoom);

    void applyTransform();

    void rotate(const core::Angle dx, const core::Angle dy, const core::Angle dz)
    {
        m_state.rotation.X += dx;
        m_state.rotation.Y += dy;
        m_state.rotation.Z += dz;
    }

    void move(const core::Length dx, const core::Length dy, const core::Length dz)
    {
        m_state.position.position.X += dx;
        m_state.position.position.Y += dy;
        m_state.position.position.Z += dz;
    }

    void move(const glm::vec3& d)
    {
        m_state.position.position += core::TRVec( d );
    }

    void moveLocal(const core::TRVec d)
    {
        m_state.position.position += util::pitch( d, m_state.rotation.Y );
    }

    const Engine& getEngine() const
    {
        return *m_engine;
    }

    Engine& getEngine()
    {
        return *m_engine;
    }

    void dampenHorizontalSpeed(const float f)
    {
        m_state.speed -= (m_state.speed.retype_as<float>() * f).retype_as<core::Speed>();
    }

    virtual void patchFloor(const core::TRVec& /*pos*/, core::Length& /*y*/)
    {
    }

    virtual void patchCeiling(const core::TRVec& /*pos*/, core::Length& /*y*/)
    {
    }

    void activate();

    void deactivate();

    virtual bool triggerSwitch(core::Frame timeout) = 0;

    std::shared_ptr<audio::SourceHandle> playSoundEffect(core::SoundId id);

    bool triggerPickUp();

    bool triggerKey();

    virtual core::Angle getMovementAngle() const
    {
        return m_state.rotation.Y;
    }

    bool alignTransform(const core::TRVec& speed, const ItemNode& target)
    {
        auto targetPos = target.m_state.position.position.toRenderSystem();
        targetPos += glm::vec3{target.m_state.rotation.toMatrix() * glm::vec4{speed.toRenderSystem(), 1.0f}};

        return alignTransformClamped( core::TRVec{targetPos}, target.m_state.rotation, 16_len, 364_au );
    }

    void updateLighting()
    {
        m_lighting.bind( *getNode() );
        auto tmp = m_state.position;
        tmp.position += getBoundingBox().getCenter();
        m_lighting.updateDynamic( m_state.shade, tmp );
    }

    virtual loader::file::BoundingBox getBoundingBox() const = 0;

    virtual void collide(LaraNode& /*other*/, CollisionInfo& /*collisionInfo*/)
    {
    }

    void kill();

    virtual YAML::Node save() const;

    virtual void load(const YAML::Node& n);

    void playShotMissed(const core::RoomBoundPosition& pos);

    boost::optional<core::Length> getWaterSurfaceHeight() const;

protected:
    bool alignTransformClamped(const core::TRVec& targetPos,
                               const core::TRRotation& targetRot,
                               const core::Length maxDistance,
                               const core::Angle& maxAngle)
    {
        auto d = targetPos - m_state.position.position;
        const auto dist = d.length();
        if( maxDistance < dist )
        {
            move( maxDistance.retype_as<float>().get() * normalize( d.toRenderSystem() ) );
        }
        else
        {
            m_state.position.position = targetPos;
        }

        core::TRRotation phi = targetRot - m_state.rotation;
        m_state.rotation.X += util::clamp( phi.X, -maxAngle, maxAngle );
        m_state.rotation.Y += util::clamp( phi.Y, -maxAngle, maxAngle );
        m_state.rotation.Z += util::clamp( phi.Z, -maxAngle, maxAngle );

        phi = targetRot - m_state.rotation;
        d = targetPos - m_state.position.position;

        return abs( phi.X ) < 1_au && abs( phi.Y ) < 1_au && abs( phi.Z ) < 1_au
               && d.X == 0_len && d.Y == 0_len && d.Z == 0_len;
    }
};


class ModelItemNode
        : public ItemNode
{
protected:
    std::shared_ptr<SkeletalModelNode> m_skeleton;

public:
    ModelItemNode(
            const gsl::not_null<Engine*>& engine,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            bool hasUpdateFunction,
            const loader::file::SkeletalModelType& animatedModel);

    ModelItemNode(const ModelItemNode&) = delete;

    ModelItemNode(ModelItemNode&&) = delete;

    ModelItemNode& operator=(const ModelItemNode&) = delete;

    ModelItemNode& operator=(ModelItemNode&&) = delete;

    ~ModelItemNode() override
    {
        if( m_skeleton != nullptr )
        {
            setParent( m_skeleton, nullptr );
        }
    }

    std::shared_ptr<gameplay::Node> getNode() const override
    {
        return m_skeleton;
    }

    const std::shared_ptr<SkeletalModelNode>& getSkeleton() const
    {
        return m_skeleton;
    }

    bool triggerSwitch(const core::Frame timeout) override
    {
        if( m_state.triggerState != TriggerState::Deactivated )
        {
            return false;
        }

        if( m_state.current_anim_state == 0_as && timeout > 0_frame )
        {
            // switch has a timer
            m_state.timer = timeout;
            m_state.triggerState = TriggerState::Active;
        }
        else
        {
            deactivate();
            m_state.triggerState = TriggerState::Inactive;
        }

        return true;
    }

    void update() override;

    void applyMovement(bool forLara);

    loader::file::BoundingBox getBoundingBox() const override;

    bool isNear(const ModelItemNode& other, core::Length radius) const;

    bool isNear(const Particle& other, core::Length radius) const;

    bool testBoneCollision(const ModelItemNode& other);

    void enemyPush(LaraNode& lara, CollisionInfo& collisionInfo, bool enableSpaz, bool withXZCollRadius);

    gsl::not_null<std::shared_ptr<Particle>> emitParticle(const core::TRVec& localPosition,
                                                          size_t boneIndex,
                                                          gsl::not_null<std::shared_ptr<Particle>> (* generate)(
                                                                  Engine& engine,
                                                                  const core::RoomBoundPosition& pos,
                                                                  core::Speed speed,
                                                                  core::Angle angle));

    void load(const YAML::Node& n) override;

    YAML::Node save() const override;
};


class SpriteItemNode
        : public ItemNode
{
private:
    std::shared_ptr<gameplay::Node> m_node;

public:
    SpriteItemNode(
            const gsl::not_null<Engine*>& engine,
            const std::string& name,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            bool hasUpdateFunction,
            const loader::file::Sprite& sprite,
            const gsl::not_null<std::shared_ptr<gameplay::Material>>& material);

    SpriteItemNode(const SpriteItemNode&) = delete;

    SpriteItemNode(SpriteItemNode&&) = delete;

    SpriteItemNode& operator=(const SpriteItemNode&) = delete;

    SpriteItemNode& operator=(SpriteItemNode&&) = delete;

    ~SpriteItemNode() override
    {
        if( m_node != nullptr )
        {
            setParent( m_node, nullptr );
        }
    }

    bool triggerSwitch(core::Frame) override
    {
        BOOST_THROW_EXCEPTION( std::runtime_error( "triggerSwitch called on sprite" ) );
    }

    std::shared_ptr<gameplay::Node> getNode() const override
    {
        return m_node;
    }

    void update() override
    {
        // TODO
    }

    loader::file::BoundingBox getBoundingBox() const override
    {
        loader::file::BoundingBox bb;
        bb.minX = bb.maxX = m_state.position.position.X;
        bb.minY = bb.maxY = m_state.position.position.Y;
        bb.minZ = bb.maxZ = m_state.position.position.Z;
        return bb;
    }
};
}
}
