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
                      const gsl::not_null<loader::Item*>& item,
                      const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, true, 0x30, animatedModel )
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                if( updateTriggerTimeout( deltaTime ) )
                {
                    if( getCurrentState() != 0 )
                    {
                        //! @todo Restore original sector data
                    }
                    else
                    {
                        setTargetState( 1 );
                    }
                }
                else
                {
                    if( getCurrentState() == 1 )
                    {
                        setTargetState( 0 );
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
