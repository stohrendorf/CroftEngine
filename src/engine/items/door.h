#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class Door final : public ModelItemNode
        {
        public:
            Door(const gsl::not_null<level::Level*>& level,
                 const std::string& name,
                 const gsl::not_null<const loader::Room*>& room,
                 const core::Angle& angle,
                 const core::TRCoordinates& position,
                 const floordata::ActivationState& activationState,
                 int16_t darkness,
                 const loader::AnimatedModel& animatedModel)
                : ModelItemNode(level, name, room, angle, position, activationState, true, SaveHitpoints | SaveFlags, darkness, animatedModel)
            {
            }


            void update() override
            {
                if( updateActivationTimeout() )
                {
                    if( getNode()->getCurrentState() != 0 )
                    {
                        //! @todo Restore original sector data
                    }
                    else
                    {
                        getNode()->setTargetState(1);
                    }
                }
                else
                {
                    if( getNode()->getCurrentState() == 1 )
                    {
                        getNode()->setTargetState(0);
                        ModelItemNode::update();
                        return;
                    }
                    //! @todo Patch original sector data with blocking heights
                }

                ModelItemNode::update();
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }
        };
    }
}
