#include "pickupitem.h"

#include "level/level.h"
#include "engine/laranode.h"


namespace engine
{
    namespace items
    {
        bool alignTransformClamped(ItemNode& which, const glm::vec3& targetPos, const core::TRRotation& targetRot, float maxDistance, const core::Angle& maxAngle)
        {
            auto d = targetPos - which.getPosition().toRenderSystem();
            const auto dist = glm::length(d);
            if( maxDistance < dist )
            {
                which.move(maxDistance * glm::normalize(d));
            }
            else
            {
                which.setPosition(core::ExactTRCoordinates(targetPos));
            }

            core::TRRotation phi = targetRot - which.getRotation();
            if( phi.X > maxAngle )
                which.addXRotation(maxAngle);
            else if( phi.X < -maxAngle )
                which.addXRotation(-maxAngle);
            else
                which.addXRotation(phi.X);
            if( phi.Y > maxAngle )
                which.addYRotation(maxAngle);
            else if( phi.Y < -maxAngle )
                which.addYRotation(-maxAngle);
            else
                which.addYRotation(phi.Y);
            if( phi.Z > maxAngle )
                which.addZRotation(maxAngle);
            else if( phi.Z < -maxAngle )
                which.addZRotation(-maxAngle);
            else
                which.addZRotation(phi.Z);

            phi = targetRot - which.getRotation();
            d = targetPos - which.getPosition().toRenderSystem();

            return abs(phi.X) < 1_au && abs(phi.Y) < 1_au && abs(phi.Z) < 1_au
                   && abs(d.x) < 1 && abs(d.y) < 1 && abs(d.z) < 1;
        }


        bool alignTransform(const glm::vec3& trSpeed, const ItemNode& target, ItemNode& which)
        {
            const auto speed = trSpeed / 16384.0f;
            auto targetRot = target.getRotation().toMatrix();
            auto targetPos = target.getPosition().toRenderSystem();
            targetPos += glm::vec3(glm::vec4(speed, 0) * targetRot);

            return alignTransformClamped(which, targetPos, target.getRotation(), 16, 364_au);
        }


        void setRelativeOrientedPosition(const core::ExactTRCoordinates& offset, const ItemNode& target, ItemNode& which)
        {
            which.setRotation(target.getRotation());

            auto r = target.getRotation().toMatrix();
            which.move(glm::vec3(glm::vec4(offset.toRenderSystem(), 0) * r));
        }

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

                if( lara.getCurrentState() == LaraStateId::PickUp )
                {
                    if( core::toFrame(lara.getCurrentTime()) == 2970 )
                    {
                        // TODO: Remove item from room, handle pick up

                        m_flags2_04_ready = true;
                    }
                }
                else if( getLevel().m_inputHandler->getInputState().action && lara.getCurrentState() == LaraStateId::UnderwaterStop && alignTransform(aimSpeed, *this, lara) )
                {
                    do
                    {
                        lara.setTargetState(LaraStateId::PickUp);
                        lara.advanceFrame();
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

                if( lara.getCurrentState() == LaraStateId::PickUp )
                {
                    if( core::toFrame(lara.getCurrentTime()) == 3443 )
                    {
                        if( m_shotgun )
                        {
                            const auto& shotgunLara = *getLevel().m_animatedModels[2];
                            BOOST_ASSERT(shotgunLara.boneCount == lara.getChildCount());

                            lara.getChild(7)->setDrawable(getLevel().getModel(getLevel().m_meshIndices[shotgunLara.firstMesh + 7]));
                        }

                        m_flags2_04_ready = true;

                        // TODO: Remove item from room, handle pick up
                    }
                }
                else if( getLevel().m_inputHandler->getInputState().action && lara.getHandStatus() == 0 && !lara.isFalling() && lara.getCurrentState() == LaraStateId::Stop )
                {
                    setRelativeOrientedPosition(core::ExactTRCoordinates{0, 0, -100.0f}, *this, lara);

                    // TODO: position Lara
                    do
                    {
                        lara.setTargetState(LaraStateId::PickUp);
                        lara.advanceFrame();
                    } while( lara.getCurrentAnimState() != LaraStateId::PickUp );
                    lara.setTargetState(LaraStateId::Stop);
                    lara.setHandStatus(1);
                }
            }
        }
    }
}
