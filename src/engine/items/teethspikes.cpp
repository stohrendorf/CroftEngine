#include "teethspikes.h"

#include "engine/laranode.h"

void engine::items::TeethSpikes::collide(engine::LaraNode& lara, engine::CollisionInfo& collisionInfo)
{
    if( lara.m_state.health >= 0 && isNear( lara, collisionInfo.collisionRadius ) && testBoneCollision( lara ) )
    {
        int bloodSplats = util::rand15( 2 );
        if( !lara.m_state.falling )
        {
            if( lara.m_state.speed < 30 )
            {
                return;
            }
        }
        else
        {
            if( lara.m_state.fallspeed > 0 )
            {
                // immediate death when falling into the spikes
                bloodSplats = 20;
                lara.m_state.health = -1;
            }
        }
        lara.m_state.health -= 15;
        while( bloodSplats-- > 0 )
        {
            auto fx = createBloodSplat( getLevel(),
                                        core::RoomBoundPosition{
                                                lara.m_state.position.room,
                                                lara.m_state.position.position + core::TRVec{
                                                        util::rand15s( 128 ),
                                                        -util::rand15( 512 ),
                                                        util::rand15s( 128 )
                                                }
                                        },
                                        20, util::rand15( +180_deg ) );
            getLevel().m_particles.emplace_back( fx );
        }
        if( lara.m_state.health <= 0 )
        {
            lara.m_state.anim = &getLevel().m_animations[static_cast<int>(loader::AnimationId::SPIKED)];
            lara.m_state.frame_number = 3887;
            lara.m_state.current_anim_state = static_cast<int>(loader::LaraStateId::Death);
            lara.m_state.goal_anim_state = static_cast<int>(loader::LaraStateId::Death);
            lara.m_state.falling = false;
            lara.m_state.position.position.Y = m_state.position.position.Y;
        }
    }
}
