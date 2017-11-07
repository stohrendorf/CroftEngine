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
                 const loader::Item& item,
                 const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, item, true, SaveHitpoints | SaveFlags, animatedModel)
            {
            }


            void update() override
            {
                if( m_state.updateActivationTimeout() )
                {
                    if( getCurrentState() != 0 )
                    {
                        //! @todo Restore original sector data
                    }
                    else
                    {
                        m_state.goal_anim_state = 1;
                    }
                }
                else
                {
                    if( getCurrentState() == 1 )
                    {
                        m_state.goal_anim_state = 0;
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
