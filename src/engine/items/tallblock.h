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
                : ItemNode(level, name, room, angle, position, activationState, true, 0x34, darkness, animatedModel)
            {
                loader::Room::patchHeightsForBlock(*this, -2 * loader::SectorSize);
            }


            void updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/) override
            {
                if( updateActivationTimeout(deltaTime) )
                {
                    if( getCurrentState() == 0 )
                    {
                        loader::Room::patchHeightsForBlock(*this, 2 * loader::SectorSize);
                        setTargetState(1);
                    }
                }
                else
                {
                    if( getCurrentState() == 1 )
                    {
                        loader::Room::patchHeightsForBlock(*this, 2 * loader::SectorSize);
                        setTargetState(0);
                    }
                }
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;
        };
    }
}
