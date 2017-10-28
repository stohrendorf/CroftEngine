#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class CollapsibleFloor final : public ModelItemNode
        {
        public:
            CollapsibleFloor(const gsl::not_null<level::Level*>& level,
                             const std::string& name,
                             const gsl::not_null<const loader::Room*>& room,
                             const core::Angle& angle,
                             const core::TRCoordinates& position,
                             const floordata::ActivationState& activationState,
                             int16_t darkness,
                             const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags | NonLot, darkness, animatedModel)
            {
            }


            void update() override;


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void patchFloor(const core::TRCoordinates& pos, int& y) override
            {
                if( pos.Y > getPosition().Y - 512 )
                    return;

                if( getCurrentState() != 0 && getSkeleton()->getCurrentState() != 1 )
                    return;

                y = getPosition().Y - 512;
            }


            void patchCeiling(const core::TRCoordinates& pos, int& y) override
            {
                if( pos.Y <= getPosition().Y - 512 )
                    return;

                if( getCurrentState() != 0 && getSkeleton()->getCurrentState() != 1 )
                    return;

                y = getPosition().Y - 256;
            }
        };
    }
}
