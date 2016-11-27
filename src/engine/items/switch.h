#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class Item_55_Switch final : public ItemNode
        {
        public:
            Item_55_Switch(const gsl::not_null<level::Level*>& level,
                           const std::string& name,
                           const gsl::not_null<const loader::Room*>& room,
                           const gsl::not_null<loader::Item*>& item,
                           const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, true, 0x30, animatedModel )
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                if( !updateTriggerTimeout( deltaTime ) )
                {
                    setTargetState( 1 );
                    m_triggerTimeout = std::chrono::microseconds::zero();
                }
            }


            void onInteract(LaraNode& lara) override;


            void onFrameChanged(FrameChangeType frameChangeType) override
            {
                m_itemFlags |= ActivationMask;

                ItemNode::onFrameChanged( frameChangeType );
            }
        };
    }
}
