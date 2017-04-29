#include "pickupitem.h"

#include "level/level.h"
#include "engine/laranode.h"


namespace engine
{
    namespace items
    {
        void PickupItem::onInteract(LaraNode& lara)
        {
            setYRotation(lara.getRotation().Y);
            setZRotation(0_deg);

            if( lara.isInWater() )
            {
                if( !lara.isDiving() )
                    return;

                static const InteractionLimits limits{
                    gameplay::BoundingBox{core::TRCoordinates{-512, -512, -512}.toRenderSystem(),
                        core::TRCoordinates{512, 512, 512}.toRenderSystem()},
                    {-45_deg, -45_deg, -45_deg},
                    {+45_deg, +45_deg, +45_deg}
                };

                if( !limits.canInteract(*this, lara) )
                    return;

                static const glm::vec3 aimSpeed{0, -200.0f, -350.0f};

                if( lara.getCurrentAnimState() == LaraStateId::PickUp )
                {
                    if( lara.getCurrentFrame() == 2970 )
                    {
                        // TODO: Remove item from room, handle pick up

                        m_triggerState = engine::items::TriggerState::Locked;
                    }
                }
                else if( getLevel().m_inputHandler->getInputState().action && lara.getCurrentAnimState() == LaraStateId::UnderwaterStop && lara.alignTransform(aimSpeed, *this) )
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
                    gameplay::BoundingBox{core::TRCoordinates{-256, -100, -256}.toRenderSystem(),
                        core::TRCoordinates{256, 100, 100}.toRenderSystem()},
                    {-10_deg, 0_deg, 0_deg},
                    {+10_deg, 0_deg, 0_deg}
                };

                if( !limits.canInteract(*this, lara) )
                    return;

                if( lara.getCurrentAnimState() == LaraStateId::PickUp )
                {
                    if( lara.getCurrentFrame() == 3443 )
                    {
                        if( m_shotgun )
                        {
                            const auto& shotgunLara = *getLevel().m_animatedModels[2];
                            BOOST_ASSERT(shotgunLara.boneCount == lara.getChildCount());

                            lara.getChild(7)->setDrawable(getLevel().getModel(getLevel().m_meshIndices[shotgunLara.firstMesh + 7]));
                        }

                        m_triggerState = engine::items::TriggerState::Locked;

                        // TODO: Remove item from room, handle pick up
                    }
                }
                else if( getLevel().m_inputHandler->getInputState().action && lara.getHandStatus() == 0 && !lara.isFalling() && lara.getCurrentAnimState() == LaraStateId::Stop )
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
