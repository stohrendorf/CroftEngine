#pragma once

#include "audio/sourcehandle.h"
#include "core/boundingbox.h"
#include "engine/floordata/floordata.h"
#include "engine/skeletalmodelnode.h"

#include <set>
#include <scriptengine/lua/sol.hpp>
#include <engine/collisioninfo.h>

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

    bool canInteract(const ItemNode& item, const LaraNode& lara) const;
};


enum class TriggerState
{
    Disabled,
    Enabled,
    Activated,
    Locked
};


struct ItemState final
{
    ~ItemState();

    int32_t floor = 0;
    int32_t touch_bits = 0;
    uint32_t mesh_bits = 0;
    uint16_t object_number = 0;
    uint16_t current_anim_state = 0;
    uint16_t goal_anim_state = 0;
    uint16_t required_anim_state = 0;
    uint16_t anim_number = 0;
    uint16_t frame_number = 0;
    int16_t speed = 0;
    int16_t fallspeed = 0;
    int16_t health = 0;
    const loader::Box* box_number = nullptr;
    int16_t timer = 0;
    floordata::ActivationState activationState;
    int16_t shade = 0;
    TriggerState triggerState = TriggerState::Disabled;
    union
    {
        uint16_t flags = 0;
        struct
        {
            bool loaded
                    : 1;
            bool intelligent
                    : 1;
            bool non_lot
                    : 1;
            bool falling
                    : 1;
            bool is_hit
                    : 1;
            bool collidable
                    : 1;
            bool already_looked_at
                    : 1;
            bool dynamic_light
                    : 1;
        };
    };

    core::TRRotation rotation;
    core::RoomBoundPosition position;

    std::shared_ptr<ai::CreatureInfo> creatureInfo;

    bool updateActivationTimeout()
    {
        if( !activationState.isFullyActivated() )
        {
            return activationState.isInverted();
        }

        if( timer == 0 )
        {
            return !activationState.isInverted();
        }

        if( timer < 0 )
        {
            return activationState.isInverted();
        }

        --timer;
        if( timer <= 0 )
        {
            timer = -1;
        }

        return !activationState.isInverted();
    }

    bool stalkBox(const level::Level& lvl, const loader::Box* box) const;

    bool isInsideZoneButNotInBox(const level::Level& lvl, int16_t zoneId, const loader::Box* box) const;

    bool inSameQuadrantAsBoxRelativeToLara(const level::Level& lvl, const loader::Box* box) const;

    void initCreatureInfo(const level::Level& lvl);

    void collectZoneBoxes(const level::Level& lvl);

    const loader::Sector* getCurrentSector() const
    {
        Expects( position.room != nullptr );
        return position.room->getSectorByAbsolutePosition( position.position );
    }

    static sol::usertype<ItemState> userType();
};


class ItemNode
{
    gsl::not_null<level::Level*> const m_level;

    std::set<std::weak_ptr<audio::SourceHandle>, audio::WeakSourceHandleLessComparator> m_sounds;

    void updateSounds();

public:
    using Characteristics = uint8_t;
    static const constexpr Characteristics Intelligent = 0x02;
    static const constexpr Characteristics NonLot = 0x04;
    static const constexpr Characteristics SavePosition = 0x08;
    static const constexpr Characteristics SaveHitpoints = 0x10;
    static const constexpr Characteristics SaveFlags = 0x20;
    static const constexpr Characteristics SaveAnim = 0x40;
    static const constexpr Characteristics SemiTransparent = 0x40;

    ItemState m_state;

    bool m_isActive = false;

    const bool m_hasProcessAnimCommandsOverride;
    const Characteristics m_characteristics;

    struct Lighting
    {
        glm::vec3 position;
        float base;
        float baseDiff;
    };

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
             bool hasProcessAnimCommandsOverride,
             Characteristics characteristics);

    virtual ~ItemNode() = default;

    virtual void update() = 0;

    virtual std::shared_ptr<gameplay::Node> getNode() const = 0;

    void setCurrentRoom(const loader::Room* newRoom);

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
        m_state.position.position += core::TRCoordinates( d );
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

    virtual void patchFloor(const core::TRCoordinates& /*pos*/, int& /*y*/)
    {
    }

    virtual void patchCeiling(const core::TRCoordinates& /*pos*/, int& /*y*/)
    {
    }

    void activate();

    void deactivate();

    virtual bool triggerSwitch(uint16_t arg) = 0;

    std::shared_ptr<audio::SourceHandle> playSoundEffect(int id);

    bool triggerPickUp()
    {
        if( m_state.triggerState != engine::items::TriggerState::Locked )
        {
            return false;
        }

        m_state.triggerState = TriggerState::Activated;
        return true;
    }

    bool triggerKey();

    virtual core::Angle getMovementAngle() const
    {
        return m_state.rotation.Y;
    }

    bool alignTransform(const glm::vec3& trSpeed, const ItemNode& target)
    {
        const auto speed = trSpeed / 16384.0f;
        const auto targetRot = target.m_state.rotation.toMatrix();
        auto targetPos = target.m_state.position.position.toRenderSystem();
        targetPos += glm::vec3( glm::vec4( speed, 0 ) * targetRot );

        return alignTransformClamped( targetPos, target.m_state.rotation, 16, 364_au );
    }

    void setRelativeOrientedPosition(const core::TRCoordinates& offset, const ItemNode& target)
    {
        m_state.rotation = target.m_state.rotation;

        const auto r = target.m_state.rotation.toMatrix();
        move( glm::vec3( glm::vec4( offset.toRenderSystem(), 0 ) * r ) );
    }

    void updateLighting()
    {
        m_lighting.baseDiff = 0;

        if( m_state.shade >= 0 )
        {
            m_lighting.base = (m_state.shade - 4096) / 8192.0f;
            if( m_lighting.base == 0 )
            {
                m_lighting.base = 1;
            }
            return;
        }

        const auto roomAmbient = 1 - m_state.position.room->ambientDarkness / 8191.0f;
        BOOST_ASSERT( roomAmbient >= 0 && roomAmbient <= 1 );
        m_lighting.base = roomAmbient;

        if( m_state.position.room->lights.empty() )
        {
            m_lighting.base = 1;
            m_lighting.baseDiff = 0;
            return;
        }

        float maxBrightness = 0;
        const auto bboxCtr = m_state.position.position + getBoundingBox().getCenter();
        for( const auto& light : m_state.position.room->lights )
        {
            auto radiusSq = light.radius / 4096.0f;
            radiusSq *= radiusSq;

            auto distanceSq = bboxCtr.distanceTo( light.position ) / 4096.0f;
            distanceSq *= distanceSq;

            const auto lightBrightness = roomAmbient + radiusSq * light.getBrightness() / (radiusSq + distanceSq);
            if( lightBrightness > maxBrightness )
            {
                maxBrightness = lightBrightness;
                m_lighting.position = light.position.toRenderSystem();
            }
        }

        m_lighting.base = (roomAmbient + maxBrightness) / 2;
        m_lighting.baseDiff = (maxBrightness - m_lighting.base);

        if( m_lighting.base == 0 && m_lighting.baseDiff == 0 )
        {
            m_lighting.base = 1;
        }
    }

    static const ItemNode* findBaseItemNode(const gameplay::Node& node)
    {
        const ItemNode* item = nullptr;

        auto n = &node;
        while( true )
        {
            item = dynamic_cast<const ItemNode*>(n);

            if( item != nullptr || n->getParent().expired() )
            {
                break;
            }

            n = n->getParent().lock().get();
        }

        return item;
    }

    static void lightBaseBinder(const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform)
    {
        const ItemNode* item = findBaseItemNode( node );

        if( item == nullptr )
        {
            uniform.set( 1.0f );
            return;
        }

        uniform.set( item->m_lighting.base );
    };

    static void lightBaseDiffBinder(const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform)
    {
        const ItemNode* item = findBaseItemNode( node );

        if( item == nullptr )
        {
            uniform.set( 1.0f );
            return;
        }

        uniform.set( item->m_lighting.baseDiff );
    };

    static void lightPositionBinder(const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform)
    {
        const ItemNode* item = findBaseItemNode( node );

        if( item == nullptr )
        {
            static const glm::vec3 invalidPos{std::numeric_limits<float>::quiet_NaN()};
            uniform.set( invalidPos );
            return;
        }

        uniform.set( item->m_lighting.position );
    };

    virtual BoundingBox getBoundingBox() const = 0;

    virtual void collide(LaraNode& other, CollisionInfo& collisionInfo)
    {
    }

protected:
    bool alignTransformClamped(const glm::vec3& targetPos, const core::TRRotation& targetRot, const int maxDistance,
                               const core::Angle& maxAngle)
    {
        auto d = targetPos - m_state.position.position.toRenderSystem();
        const auto dist = glm::length( d );
        if( maxDistance < dist )
        {
            move( static_cast<float>(maxDistance) * glm::normalize( d ) );
        }
        else
        {
            const core::TRCoordinates& pos = core::TRCoordinates( targetPos );
            m_state.position.position = pos;
        }

        core::TRRotation phi = targetRot - m_state.rotation;
        if( phi.X > maxAngle )
        {
            m_state.rotation.X += maxAngle;
        }
        else if( phi.X < -maxAngle )
        {
            m_state.rotation.X += -maxAngle;
        }
        else
        {
            m_state.rotation.X += phi.X;
        }
        if( phi.Y > maxAngle )
        {
            m_state.rotation.Y += maxAngle;
        }
        else if( phi.Y < -maxAngle )
        {
            m_state.rotation.Y += -maxAngle;
        }
        else
        {
            m_state.rotation.Y += phi.Y;
        }
        if( phi.Z > maxAngle )
        {
            m_state.rotation.Z += maxAngle;
        }
        else if( phi.Z < -maxAngle )
        {
            m_state.rotation.Z += -maxAngle;
        }
        else
        {
            m_state.rotation.Z += phi.Z;
        }

        phi = targetRot - m_state.rotation;
        d = targetPos - m_state.position.position.toRenderSystem();

        return abs( phi.X ) < 1_au && abs( phi.Y ) < 1_au && abs( phi.Z ) < 1_au
               && abs( d.x ) < 1 && abs( d.y ) < 1 && abs( d.z ) < 1;
    }
};


class ModelItemNode
        : public ItemNode
{
    std::shared_ptr<SkeletalModelNode> m_skeleton;

public:
    ModelItemNode(
            const gsl::not_null<level::Level*>& level,
            const std::string& name,
            const gsl::not_null<const loader::Room*>& room,
            const loader::Item& item,
            bool hasProcessAnimCommandsOverride,
            Characteristics characteristics,
            const loader::SkeletalModelType& animatedModel);

    std::shared_ptr<gameplay::Node> getNode() const override
    {
        return m_skeleton;
    }

    const std::shared_ptr<SkeletalModelNode>& getSkeleton() const
    {
        return m_skeleton;
    }

    bool triggerSwitch(uint16_t arg) override
    {
        if( m_state.triggerState != engine::items::TriggerState::Activated )
        {
            return false;
        }

        if( m_state.current_anim_state != 0 || engine::floordata::ActivationState{arg}.isLocked() )
        {
            deactivate();
            m_state.triggerState = TriggerState::Disabled;
        }
        else
        {
            m_state.timer = engine::floordata::ActivationState::extractTimeout( arg );
            m_state.triggerState = TriggerState::Enabled;
        }

        return true;
    }

    void update() override;

    void applyMovement(bool forLara);

    BoundingBox getBoundingBox() const override;

    bool isNear(const ModelItemNode& other, int radius) const;
    bool testBoneCollision(const ModelItemNode& other);

    void enemyPush(LaraNode& other, CollisionInfo& collisionInfo, bool enableSpaz, bool withXZCollRadius);
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
            bool hasProcessAnimCommandsOverride,
            Characteristics characteristics,
            const loader::Sprite& sprite,
            const std::shared_ptr<gameplay::Material>& material,
            const std::vector<std::shared_ptr<gameplay::gl::Texture>>& textures);

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

    BoundingBox getBoundingBox() const override
    {
        BoundingBox bb;
        bb.minX = bb.maxX = m_state.position.position.X;
        bb.minY = bb.maxY = m_state.position.position.Y;
        bb.minZ = bb.maxZ = m_state.position.position.Z;
        return bb;
    }
};
}
}
