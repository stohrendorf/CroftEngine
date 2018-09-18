#include "pickupitem.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
void PickupItem::collide(LaraNode& lara, CollisionInfo& collisionInfo)
{
    const core::Angle y = lara.m_state.rotation.Y;
    m_state.rotation.Y = y;
    m_state.rotation.Z = 0_deg;

    if( lara.isInWater() )
    {
        if( !lara.isDiving() )
        {
            return;
        }

        static const InteractionLimits limits{
                core::BoundingBox{{-512, -512, -512},
                                  {512,  512,  512}},
                {-45_deg, -45_deg, -45_deg},
                {+45_deg, +45_deg, +45_deg}
        };

        if( !limits.canInteract( m_state, lara.m_state ) )
        {
            return;
        }

        static const glm::vec3 aimSpeed{0, -200.0f, -350.0f};

        if( lara.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( lara.m_state.frame_number == 2970 )
            {
                m_state.triggerState = engine::items::TriggerState::Invisible;
                getNode()->setVisible( false );
                getLevel().addInventoryItem( m_state.object_number );
            }
        }
        else if( getLevel().m_inputHandler->getInputState().action
                 && lara.getCurrentAnimState() == LaraStateId::UnderwaterStop
                 && lara.alignTransform( aimSpeed, *this ) )
        {
            do
            {
                lara.setGoalAnimState( LaraStateId::PickUp );
                lara.updateImpl();
            } while( lara.getCurrentAnimState() != LaraStateId::PickUp );
            lara.setGoalAnimState( LaraStateId::UnderwaterStop );
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

        if( !limits.canInteract( m_state, lara.m_state ) )
        {
            return;
        }

        if( lara.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( lara.m_state.frame_number == 3443 )
            {
                if( m_state.object_number == engine::TR1ItemId::ShotgunSprite )
                {
                    const auto& shotgunLara = *getLevel().m_animatedModels[engine::TR1ItemId::LaraShotgunAnim];
                    BOOST_ASSERT( shotgunLara.nmeshes == lara.getNode()->getChildren().size() );

                    lara.getNode()->getChild( 7 )->setDrawable( shotgunLara.models[7].get() );
                }

                m_state.triggerState = engine::items::TriggerState::Invisible;
                getNode()->setVisible( false );
                getLevel().addInventoryItem( m_state.object_number );
            }
        }
        else
        {
            if( getLevel().m_inputHandler->getInputState().action
                && lara.getHandStatus() == HandStatus::None
                && !lara.m_state.falling
                && lara.getCurrentAnimState() == LaraStateId::Stop )
            {
                lara.alignForInteraction( core::TRVec{0, 0, -100}, m_state );

                do
                {
                    lara.setGoalAnimState( LaraStateId::PickUp );
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
