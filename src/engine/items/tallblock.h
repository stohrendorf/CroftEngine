#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class TallBlock final : public ItemNode
        {
        public:
            TallBlock(const gsl::not_null<level::Level*>& level,
                      const std::string& name,
                      const gsl::not_null<const loader::Room*>& room,
                      const core::Angle& angle,
                      const core::ExactTRCoordinates& position,
                      const floordata::ActivationState& activationState,
                      int16_t darkness,
                      const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags | NonLot, darkness, animatedModel)
            {
                loader::Room::patchHeightsForBlock(*this, -2 * loader::SectorSize);
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void update(const std::chrono::microseconds& deltaTime) override;
        };
    }
}
