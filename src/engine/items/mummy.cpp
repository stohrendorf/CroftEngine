#include <engine/script/reflection.h>
#include "mummy.h"

#include "engine/laranode.h"

namespace engine
{
namespace items
{
Mummy::Mummy(const gsl::not_null<Engine*>& engine, const gsl::not_null<const loader::file::Room*>& room,
             const loader::file::Item& item, const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{engine, room, item, true, animatedModel}
{
    m_state.health = core::Health{static_cast<core::Health::type>(engine->getScriptEngine()["getObjectInfo"]
            .call<engine::script::ObjectInfo>( m_state.type.get() ).hit_points)};
}

void Mummy::update()
{
    if( m_state.current_anim_state == 1_as )
    {
        auto head = angleFromAtan(
                getEngine().getLara().m_state.position.position.X - m_state.position.position.X,
                getEngine().getLara().m_state.position.position.Z - m_state.position.position.Z );
        head = util::clamp( head - m_state.rotation.Y, -90_deg, +90_deg );
        m_headRotation += util::clamp( head - m_headRotation, -5_deg, +5_deg );
        getSkeleton()->patchBone( 3, core::TRRotation{0_deg, m_headRotation, 0_deg}.toMatrix() );

        if( m_state.health <= 0_hp || m_state.touch_bits.any() )
        {
            m_state.goal_anim_state = 2_as;
        }
    }

    ModelItemNode::update();

    if( m_state.triggerState == TriggerState::Deactivated )
    {
        deactivate();
        m_state.health = -16384_hp;
    }
}

void Mummy::collide(LaraNode& lara, CollisionInfo& info)
{
    if( !isNear( lara, info.collisionRadius ) )
        return;

    if( !testBoneCollision( lara ) )
        return;

    if( !info.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableBaddiePush ) )
        return;

    enemyPush( lara, info, false, true );
}
}
}
