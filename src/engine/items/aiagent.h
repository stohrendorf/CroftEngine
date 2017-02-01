#pragma once

#include "itemnode.h"

#include "engine/ai/ai.h"


namespace engine
{
    namespace items
    {
        class AIAgent : public ItemNode
        {
        public:
            AIAgent(const gsl::not_null<level::Level*>& level,
                    const std::string& name,
                    const gsl::not_null<const loader::Room*>& room,
                    const core::Angle& angle,
                    const core::ExactTRCoordinates& position,
                    const floordata::ActivationState& activationState,
                    uint8_t characteristics,
                    int16_t darkness,
                    const loader::AnimatedModel& animatedModel,
                    uint16_t blockMask,
                    int collisionRadius)
                : ItemNode(level, name, room, angle, position, activationState, true, characteristics, darkness, animatedModel)
                , m_brain{blockMask}
                , m_collisionRadius{collisionRadius}
            {
                m_flags2_20_collidable = true;
                addYRotation(core::Angle(std::rand() % 65536));
            }


        protected:
            ai::Brain& getBrain()
            {
                return m_brain;
            }


            void rotateCreatureTilt(core::Angle angle)
            {
                const auto dz = core::Angle(4 * angle.toAU()) - getRotation().Z;
                addZRotation(util::clamp(dz, -3_deg, +3_deg));
            }


            void rotateCreatureHead(core::Angle angle)
            {
                const auto d = util::clamp(angle - m_brain.jointRotation.Y, -5_deg, +5_deg);
                m_brain.jointRotation.Y = util::clamp(d + m_brain.jointRotation.Y, -90_deg, +90_deg);
            }


            bool animateCreature(core::Angle rotationToMoveTarget, core::Angle roll);


            core::Angle rotateTowardsMoveTarget(const ai::Brain& creatureData, core::Angle maxRotationSpeed);


            int getHealth() const
            {
                return m_health;
            }


        private:

            bool anyMovingEnabledItemInReach() const;

            bool isPositionOutOfReach(const core::TRCoordinates& testPosition, int currentBoxFloor, const ai::RoutePlanner& lotInfo) const;

            uint16_t m_requiredAnimState = 0;
            ai::Brain m_brain;
            int m_health = 1000;
            const int m_collisionRadius;
        };
    }
}
