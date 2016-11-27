#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class Item_Block final : public ItemNode
        {
        public:
            Item_Block(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const gsl::not_null<loader::Item*>& item,
                       const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, true, 0x34, animatedModel )
            {
                if( !m_flags2_04_ready || !m_flags2_02_toggledOn )
                    loader::Room::patchHeightsForBlock( *this, -loader::SectorSize );
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
                if( updateTriggerTimeout( deltaTime ) )
                    setTargetState( 1 );
                else
                    setTargetState( 0 );
            }


            void onInteract(LaraNode& lara) override;

            void onFrameChanged(FrameChangeType frameChangeType) override;

        private:
            bool isOnFloor(int height) const;

            bool canPushBlock(int height, core::Axis axis) const;

            bool canPullBlock(int height, core::Axis axis) const;
        };
    }
}
