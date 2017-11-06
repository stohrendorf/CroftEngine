#include "pickupitem.h"

#include "level/level.h"
#include "engine/laranode.h"

namespace engine
{
namespace items
{
void PickupItem::onInteract(LaraNode& lara)
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

        if( !limits.canInteract(*this, lara) )
        {
            return;
        }

        static const glm::vec3 aimSpeed{0, -200.0f, -350.0f};

        if( lara.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( lara.m_state.frame_number == 2970 )
            {
                // TODO: Remove item from room, handle pick up

                m_state.triggerState = engine::items::TriggerState::Locked;
            }
        }
        else if( getLevel().m_inputHandler->getInputState().action && lara.getCurrentAnimState() == LaraStateId::UnderwaterStop &&
                 lara.alignTransform(aimSpeed, *this) )
        {
            do
            {
                lara.setTargetState(LaraStateId::PickUp);
                lara.updateImpl();
            } while( lara.getCurrentAnimState() != LaraStateId::PickUp );
            lara.setTargetState(LaraStateId::UnderwaterStop);
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

        if( !limits.canInteract(*this, lara) )
        {
            return;
        }

        if( lara.getCurrentAnimState() == LaraStateId::PickUp )
        {
            if( lara.m_state.frame_number == 3443 )
            {
                if( m_shotgun )
                {
                    const auto& shotgunLara = *getLevel().m_animatedModels[2];
                    BOOST_ASSERT(shotgunLara.boneCount == lara.getNode()->getChildCount());

                    lara.getNode()->getChild(7)->setDrawable(getLevel().getModel(getLevel().m_meshIndices[shotgunLara.firstMesh + 7]));
                }

                m_state.triggerState = engine::items::TriggerState::Locked;

                // TODO: Remove item from room, handle pick up
            }
        }
        else
        {
            if( getLevel().m_inputHandler->getInputState().action && lara.getHandStatus() == 0 && !lara.m_state.falling &&
                lara.getCurrentAnimState() == LaraStateId::Stop )
            {
                lara.setRelativeOrientedPosition(core::TRCoordinates{0, 0, -100}, *this);

                // TODO: position Lara
                do
                {
                    lara.setTargetState(LaraStateId::PickUp);
                    lara.updateImpl();
                } while( lara.getCurrentAnimState() != LaraStateId::PickUp );
                lara.setTargetState(LaraStateId::Stop);
                lara.setHandStatus(1);
            }
        }
    }
}
}
}
