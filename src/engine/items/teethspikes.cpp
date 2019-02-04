#include "teethspikes.h"

#include "engine/laranode.h"

void engine::items::TeethSpikes::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    if( lara.m_state.health >= 0_hp && isNear( lara, collisionInfo.collisionRadius ) && testBoneCollision( lara ) )
    {
        int bloodSplats = util::rand15( 2 );
        if( !lara.m_state.falling )
        {
            if( lara.m_state.speed < 30_len )
            {
                return;
            }
        }
        else
        {
            if( lara.m_state.fallspeed > 0_len )
            {
                // immediate death when falling into the spikes
                bloodSplats = 20;
                lara.m_state.health = -1_hp;
            }
        }
        lara.m_state.health -= 15_hp;
        while( bloodSplats-- > 0 )
        {
            auto fx = createBloodSplat( getLevel(),
                                        core::RoomBoundPosition{
                                                lara.m_state.position.room,
                                                lara.m_state.position.position + core::TRVec{
                                                        util::rand15s( 128_len ),
                                                        -util::rand15( 512_len ),
                                                        util::rand15s( 128_len )
                                                }
                                        },
                                        20_len, util::rand15( +180_deg ) );
            getLevel().m_particles.emplace_back( fx );
        }
        if( lara.m_state.health <= 0_hp )
        {
            lara.m_state.anim = &getLevel().m_animations[static_cast<int>(loader::AnimationId::SPIKED)];
            lara.m_state.frame_number = 3887_frame;
            lara.setCurrentAnimState( loader::LaraStateId::Death );
            lara.setGoalAnimState( loader::LaraStateId::Death );
            lara.m_state.falling = false;
            lara.m_state.position.position.Y = m_state.position.position.Y;
        }
    }
}
