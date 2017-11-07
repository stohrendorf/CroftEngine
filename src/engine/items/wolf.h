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
                 const loader::Item& item,
                 const loader::SkeletalModelType& animatedModel)
                : AIAgent(level, name, room, item, SaveHitpoints | SaveFlags | SavePosition | NonLot | Intelligent, animatedModel, 384)
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
