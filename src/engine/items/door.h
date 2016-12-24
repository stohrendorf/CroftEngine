#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Door final : public ItemNode
        {
        public:
            Door(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::ExactTRCoordinates& position,
                 const floordata::ActivationState& activationState,
                 int16_t darkness,
                 const loader::AnimatedModel& animatedModel)
                : ItemNode(level, name, room, angle, position, activationState, true, 0x30, darkness, animatedModel)
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime, const boost::optional<FrameChangeType>& /*frameChangeType*/) override
            {
                if( updateActivationTimeout(deltaTime) )
                {
                    if( getCurrentState() != 0 )
                    {
                        //! @todo Restore original sector data
                    }
                    else
                    {
                        setTargetState(1);
                    }
                }
                else
                {
                    if( getCurrentState() == 1 )
                    {
                        setTargetState(0);
                        return;
                    }
                    //! @todo Patch original sector data with blocking heights
                }
            }


            void onInteract(LaraNode& lara) override
            {
            }
        };
    }
}
