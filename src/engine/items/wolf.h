#pragma once

#include "itemnode.h"

#include "engine/ai/ai.h"

namespace engine
{
    namespace items
    {
        class Wolf final
            : public ItemNode
        {
        public:
            Wolf(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::ExactTRCoordinates& position,
                 const floordata::ActivationState& activationState,
                 int16_t darkness,
                 const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, 0x3e, darkness, animatedModel)
                  , m_brain{0x4000}
            {
                m_flags2_20_collidable = true;
                addYRotation(core::Angle(std::rand() % 65536));
            }


            void updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/) override;


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            ai::Brain m_brain;
            int m_health = 1000;
            int m_collisionRadius = 384;

        private:
            uint16_t m_requiredAnimState = 0;
        };
    }
}
