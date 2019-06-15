#include "slammingdoors.h"

#include "engine/heightinfo.h"
#include "engine/laranode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
void SlammingDoors::update()
{
    if( !m_state.updateActivationTimeout() )
    {
        m_state.goal_anim_state = 0_as;
    }
    else
    {
        m_state.goal_anim_state = 1_as;
        if( m_state.touch_bits.any() && m_state.current_anim_state == 1_as )
        {
            getEngine().getLara().m_state.health -= 400_hp;
            getEngine().getLara().m_state.is_hit = true;

            const auto itemSpheres = getSkeleton()->getBoneCollisionSpheres(
                m_state,
                *getSkeleton()->getInterpolationInfo( m_state ).getNearestFrame(),
                nullptr );

            const auto emitBlood = [&itemSpheres, this](const core::TRVec& bitePos, size_t boneId) {
              const auto position = core::TRVec{
                  glm::vec3{ translate( itemSpheres.at( boneId ).m, bitePos.toRenderSystem() )[3] }
              };

              auto blood = createBloodSplat( getEngine(), core::RoomBoundPosition{ m_state.position.room, position },
                                             m_state.speed, m_state.rotation.Y );
              getEngine().getParticles().emplace_back( std::move( blood ) );
            };

            for( auto x : { -23_len, 71_len } )
            {
                for( auto y : { 0_len, 10_len, -10_len } )
                    emitBlood( { x, y, -1718_len }, 0 );
            }
        }
    }
    ModelItemNode::update();
}

void SlammingDoors::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( m_state.triggerState == TriggerState::Active )
    {
        if( isNear( lara, collisionInfo.collisionRadius ) )
        {
            testBoneCollision( lara );
        }
    }
    else if( m_state.triggerState != TriggerState::Invisible
        && isNear( lara, collisionInfo.collisionRadius )
        && testBoneCollision( lara ) )
    {
        if( collisionInfo.policyFlags.is_set( CollisionInfo::PolicyFlags::EnableBaddiePush ) )
        {
            enemyPush( lara, collisionInfo, false, true );
        }
    }
}
}
}
