#include "pickupitem.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
void PickupItem::collide(LaraNode& other, CollisionInfo& collisionInfo)
{
    const core::Angle y = other.m_state.rotation.Y;
    m_state.rotation.Y = y;
    m_state.rotation.Z = 0_deg;

    if( other.isInWater() )
    {
        if( !other.isDiving() )
        {
            return;
        }

        static const InteractionLimits limits{
                core::BoundingBox{{-512, -512, -512},
                                  {512,  512,  512}},
                {-45_deg, -45_deg, -45_deg},
                {+45_deg, +45_deg, +45_deg}
        };

        if( !limits.canInteract( *this, other ) )
        {
            return;
        }

        static const glm::vec3 aimSpeed{0, -200.0f, -350.0f};

        if( other.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( other.m_state.frame_number == 2970 )
            {
                // TODO: Remove item from room, handle pick up

                m_state.triggerState = engine::items::TriggerState::Locked;
            }
        }
        else if( getLevel().m_inputHandler->getInputState().action
                 && other.getCurrentAnimState() == LaraStateId::UnderwaterStop &&
                 other.alignTransform( aimSpeed, *this ) )
        {
            do
            {
                other.setTargetState( LaraStateId::PickUp );
                other.updateImpl();
            } while( other.getCurrentAnimState() != LaraStateId::PickUp );
            other.setTargetState( LaraStateId::UnderwaterStop );
        }
    }
    else
    {
        static const InteractionLimits limits{
                core::BoundingBox{{-256, -100, -256},
                                  {256,  100,  100}},
                {-10_deg, 0_deg, 0_deg},
                {+10_deg, 0_deg, 0_deg}
        };

        if( !limits.canInteract( *this, other ) )
        {
            return;
        }

        if( other.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( other.m_state.frame_number == 3443 )
            {
                if( m_shotgun )
                {
                    const auto& shotgunLara = *getLevel().m_animatedModels[2];
                    BOOST_ASSERT( shotgunLara.nmeshes == other.getNode()->getChildCount() );

                    other.getNode()->getChild( 7 )
                         ->setDrawable( getLevel().getModel( getLevel().m_meshIndices[shotgunLara.frame_number + 7] ) );
                }

                m_state.triggerState = engine::items::TriggerState::Locked;

                // TODO: Remove item from room, handle pick up
            }
        }
        else
        {
            if( getLevel().m_inputHandler->getInputState().action && other.getHandStatus() == HandStatus::None
                && !other.m_state.falling &&
                other.getCurrentAnimState() == LaraStateId::Stop )
            {
                other.setRelativeOrientedPosition( core::TRCoordinates{0, 0, -100}, *this );

                // TODO: position Lara
                do
                {
                    other.setTargetState( LaraStateId::PickUp );
                    other.updateImpl();
                } while( other.getCurrentAnimState() != LaraStateId::PickUp );
                other.setTargetState( LaraStateId::Stop );
                other.setHandStatus( HandStatus::Grabbing );
            }
        }
    }
}
}
}
