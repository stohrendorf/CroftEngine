#include "pickupitem.h"

#include "engine/laranode.h"
#include "hid/inputhandler.h"

namespace engine
{
namespace items
{
void PickupItem::collide(LaraNode& lara, CollisionInfo& /*collisionInfo*/)
{
    m_state.rotation.Y = lara.m_state.rotation.Y;
    m_state.rotation.Z = 0_deg;

    if( lara.isInWater() )
    {
        if( !lara.isDiving() )
        {
            return;
        }

        static const InteractionLimits limits{
                core::BoundingBox{{-512_len, -512_len, -512_len},
                                  {512_len,  512_len,  512_len}},
                {-45_deg, -45_deg, -45_deg},
                {+45_deg, +45_deg, +45_deg}
        };

        m_state.rotation.X = -25_deg;

        if( !limits.canInteract( m_state, lara.m_state ) )
        {
            return;
        }

        static const core::TRVec aimSpeed{0_len, -200_len, -350_len};

        if( lara.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( lara.m_state.frame_number == 2970_frame )
            {
                m_state.triggerState = TriggerState::Invisible;
                getEngine().getInventory().put( m_state.type );
                setParent( getNode(), nullptr );
                m_state.collidable = false;
                return;
            }
        }
        else if( getEngine().getInputHandler().getInputState().action
                 && lara.getCurrentAnimState() == LaraStateId::UnderwaterStop
                 && lara.alignTransform( aimSpeed, *this ) )
        {
            lara.setGoalAnimState( LaraStateId::PickUp );
            do
            {
                lara.updateImpl();
            } while( lara.getCurrentAnimState() != LaraStateId::PickUp );
            lara.setGoalAnimState( LaraStateId::UnderwaterStop );
        }
    }
    else
    {
        static const InteractionLimits limits{
                core::BoundingBox{{-256_len, -100_len, -256_len},
                                  {256_len,  100_len,  100_len}},
                {-10_deg, 0_deg, 0_deg},
                {+10_deg, 0_deg, 0_deg}
        };

        m_state.rotation.X = 0_deg;

        if( !limits.canInteract( m_state, lara.m_state ) )
        {
            return;
        }

        if( lara.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( lara.m_state.frame_number == 3443_frame )
            {
                if( m_state.type == TR1ItemId::ShotgunSprite )
                {
                    const auto& shotgunLara = *getEngine().findAnimatedModelForType(TR1ItemId::LaraShotgunAnim);
                    BOOST_ASSERT(
                            gsl::narrow<size_t>( shotgunLara.meshes.size() ) == lara.getNode()->getChildren().size() );

                    lara.getNode()->getChild( 7 )->setDrawable( shotgunLara.models[7].get() );
                }

                m_state.triggerState = TriggerState::Invisible;
                getEngine().getInventory().put( m_state.type );
                setParent( getNode(), nullptr );
                m_state.collidable = false;
            }
        }
        else
        {
            if( getEngine().getInputHandler().getInputState().action
                && lara.getHandStatus() == HandStatus::None
                && !lara.m_state.falling
                && lara.getCurrentAnimState() == LaraStateId::Stop )
            {
                lara.alignForInteraction( core::TRVec{0_len, 0_len, -100_len}, m_state );

                lara.setGoalAnimState( LaraStateId::PickUp );
                do
                {
                    lara.updateImpl();
                } while( lara.getCurrentAnimState() != LaraStateId::PickUp );
                lara.setGoalAnimState( LaraStateId::Stop );
                lara.setHandStatus( HandStatus::Grabbing );
            }
        }
    }
}
}
}
