#pragma once

#include "audio/sourcehandle.h"
#include "core/boundingbox.h"
#include "engine/floordata/floordata.h"
#include "engine/skeletalmodelnode.h"
#include "engine/items_tr1.h"
#include "engine/collisioninfo.h"
#include "engine/lighting.h"

#include <set>

#include <sol.hpp>

namespace loader
{
struct Item;
}

namespace level
{
class Level;
}

namespace engine
{
class LaraNode;


class Particle;

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


struct ItemState final
{
    explicit ItemState(const gsl::not_null<const loader::Room*>& room, const TR1ItemId type)
            : type{type}
            , position{room}
    {}

    ItemState(const ItemState&) = default;

    ItemState(ItemState&&) = default;

    ItemState& operator=(const ItemState&) = default;

    ItemState& operator=(ItemState&&) = default;

    ~ItemState();

    TR1ItemId type;
    core::RoomBoundPosition position;
    core::TRRotation rotation;
    int16_t speed = 0;
    int16_t fallspeed = 0;
    uint16_t current_anim_state = 0;
    uint16_t goal_anim_state = 0;
    uint16_t required_anim_state = 0;
    const loader::Animation* anim = nullptr;
    uint16_t frame_number = 0;
    int16_t health = 0;
    TriggerState triggerState = TriggerState::Inactive;
    int16_t timer = 0;
    floordata::ActivationState activationState;
    int32_t floor = 0;
    uint32_t touch_bits = 0;
    const loader::Box* box = nullptr;
    int16_t shade = -1;
    uint32_t mesh_bits = 0;

    bool falling = false;
    bool is_hit = false;
    bool collidable = true;
    bool already_looked_at = false;
    bool dynamic_light = false;

    std::shared_ptr<ai::CreatureInfo> creatureInfo;

    YAML::Node save(const level::Level& lvl) const;

    void load(const YAML::Node& n, const level::Level& lvl);

    bool updateActivationTimeout()
    {
        if( !activationState.isFullyActivated() )
        {
            return activationState.isInverted();
        }

        if( timer == 0 )
            return !activationState.isInverted();

        if( timer < 0 )
            return activationState.isInverted();

        --timer;
        if( timer == 0 )
            timer = -1;

        return !activationState.isInverted();
    }

    bool stalkBox(const level::Level& lvl, const loader::Box& box) const;

    bool isInsideZoneButNotInBox(const level::Level& lvl, int16_t zoneId, const loader::Box& box) const;

    bool inSameQuadrantAsBoxRelativeToLara(const level::Level& lvl, const loader::Box& box) const;

    void initCreatureInfo(const level::Level& lvl);

    void collectZoneBoxes(const level::Level& lvl);

    const loader::Sector* getCurrentSector() const
    {
        return position.room->getSectorByAbsolutePosition( position.position );
    }

    static sol::usertype<ItemState>& userType();
};


class ItemNode
{
    const gsl::not_null<level::Level*> m_level;

    std::vector<std::weak_ptr<audio::SourceHandle>> m_sounds;

    void updateSounds();

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

    ItemNode(const gsl::not_null<level::Level*>& level,
             const gsl::not_null<const loader::Room*>& room,
             const loader::Item& item,
             bool hasUpdateFunction);

    ItemNode(const ItemNode&) = delete;

    ItemNode(ItemNode&&) = delete;

    ItemNode& operator=(const ItemNode&) = delete;

    ItemNode& operator=(ItemNode&&) = delete;

    virtual ~ItemNode() = default;

    virtual void update() = 0;

    virtual std::shared_ptr<gameplay::Node> getNode() const = 0;

    void setCurrentRoom(const gsl::not_null<const loader::Room*>& newRoom);

    void applyTransform();

    void rotate(const core::Angle dx, const core::Angle dy, const core::Angle dz)
    {
        m_state.rotation.X += dx;
        m_state.rotation.Y += dy;
        m_state.rotation.Z += dz;
    }

    void move(const int dx, const int dy, const int dz)
    {
        m_state.position.position.X += dx;
        m_state.position.position.Y += dy;
        m_state.position.position.Z += dz;
    }

    void move(const glm::vec3& d)
    {
        m_state.position.position += core::TRVec( d );
    }

    void moveLocal(const int dx, const int dy, const int dz)
    {
        const auto sin = m_state.rotation.Y.sin();
        const auto cos = m_state.rotation.Y.cos();
        m_state.position.position.X += dz * sin + dx * cos;
        m_state.position.position.Y += dy;
        m_state.position.position.Z += dz * cos - dx * sin;
    }

    const level::Level& getLevel() const
    {
        return *m_level;
    }

    level::Level& getLevel()
    {
        return *m_level;
    }

    void dampenHorizontalSpeed(const float f)
    {
        m_state.speed -= m_state.speed * f;
    }

    virtual void patchFloor(const core::TRVec& /*pos*/, int& /*y*/)
    {
    }

    virtual void patchCeiling(const core::TRVec& /*pos*/, int& /*y*/)
    {
    }

    void activate();

    void deactivate();

    virtual bool triggerSwitch(uint16_t timeout) = 0;

    std::shared_ptr<audio::SourceHandle> playSoundEffect(TR1SoundId id);

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

        return alignTransformClamped( core::TRVec{targetPos}, target.m_state.rotation, 16, 364_au );
    }

    void updateLighting()
    {
        m_lighting.bind( *getNode() );
        auto tmp = m_state.position;
        tmp.position += getBoundingBox().getCenter();
        m_lighting.updateDynamic( m_state.shade, tmp );
    }

    virtual loader::BoundingBox getBoundingBox() const = 0;

    virtual void collide(LaraNode& /*other*/, CollisionInfo& /*collisionInfo*/)
    {
    }

    void kill();

    virtual YAML::Node save() const;

    virtual void load(const YAML::Node& n);

    void playShotMissed(const core::RoomBoundPosition& pos);

    boost::optional<int> getWaterSurfaceHeight() const;

protected:
    bool alignTransformClamped(const core::TRVec& targetPos,
                               const core::TRRotation& targetRot,
                               const int maxDistance,
                               const core::Angle& maxAngle)
    {
        auto d = targetPos - m_state.position.position;
        const auto dist = d.length();
        if( maxDistance < dist )
        {
            move( static_cast<float>(maxDistance) * normalize( d.toRenderSystem() ) );
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
               && d.X == 0 && d.Y == 0 && d.Z == 0;
    }
};


class ModelItemNode
        : public ItemNode
{
protected:
    std::shared_ptr<SkeletalModelNode> m_skeleton;

public:
    ModelItemNode(
            const gsl::not_null<level::Level*>& level,
            const gsl::not_null<const loader::Room*>& room,
            const loader::Item& item,
            bool hasUpdateFunction,
            const loader::SkeletalModelType& animatedModel);

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

    bool triggerSwitch(const uint16_t timeout) override
    {
        if( m_state.triggerState != TriggerState::Deactivated )
        {
            return false;
        }

        if( m_state.current_anim_state == 0 && timeout > 0 )
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

    loader::BoundingBox getBoundingBox() const override;

    bool isNear(const ModelItemNode& other, int radius) const;

    bool isNear(const Particle& other, int radius) const;

    bool testBoneCollision(const ModelItemNode& other);

    void enemyPush(LaraNode& lara, CollisionInfo& collisionInfo, bool enableSpaz, bool withXZCollRadius);

    gsl::not_null<std::shared_ptr<Particle>> emitParticle(const core::TRVec& localPosition,
                                                          size_t boneIndex,
                                                          gsl::not_null<std::shared_ptr<Particle>> (* generate)(
                                                                  const level::Level& level,
                                                                  const core::RoomBoundPosition& pos,
                                                                  int16_t speed,
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
            const gsl::not_null<level::Level*>& level,
            const std::string& name,
            const gsl::not_null<const loader::Room*>& room,
            const loader::Item& item,
            bool hasUpdateFunction,
            const loader::Sprite& sprite,
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

    bool triggerSwitch(uint16_t) override
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

    loader::BoundingBox getBoundingBox() const override
    {
        loader::BoundingBox bb;
        bb.minX = bb.maxX = m_state.position.position.X;
        bb.minY = bb.maxY = m_state.position.position.Y;
        bb.minZ = bb.maxZ = m_state.position.position.Z;
        return bb;
    }
};
}
}
