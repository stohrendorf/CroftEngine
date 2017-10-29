#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"


namespace engine
{
    namespace items
    {
        class Wolf final
            : public AIAgent
        {
        public:
            Wolf(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::TRCoordinates& position,
                 uint16_t activationState,
                 int16_t darkness,
                 const loader::SkeletalModelType& animatedModel)
                : AIAgent(level, name, room, angle, position, activationState, SaveHitpoints | SaveFlags | SavePosition | NonLot | Intelligent, darkness, animatedModel, 0x4000, 384, -1024, 256, 0)
            {
            }


            void update() override;


            void onInteract(LaraNode& /*lara*/) override
            {
            }


        private:
            uint16_t m_requiredAnimState = 0;
        };
    }
}
