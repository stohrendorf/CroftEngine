#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class TallBlock final : public ModelItemNode
        {
        public:
            TallBlock(const gsl::not_null<level::Level*>& level,
                      const std::string& name,
                      const gsl::not_null<const loader::Room*>& room,
                      const core::Angle& angle,
                      const core::TRCoordinates& position,
                      const floordata::ActivationState& activationState,
                      int16_t darkness,
                      const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags | NonLot, darkness, animatedModel)
            {
                loader::Room::patchHeightsForBlock(*this, -2 * loader::SectorSize);
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void update() override;
        };
    }
}
