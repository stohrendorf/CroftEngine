#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class PickupItem final : public ItemNode
        {
        public:
            PickupItem(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const gsl::not_null<loader::Item*>& item,
                       const loader::AnimatedModel& animatedModel,
                       bool shotgun = false)
                : ItemNode(level, name, room, item, true, 0x30, animatedModel)
                , m_shotgun{shotgun}
            {
            }


            void updateImpl(const std::chrono::microseconds& deltaTime) override
            {
            }


            void onInteract(LaraNode& lara) override;


            void onFrameChanged(FrameChangeType frameChangeType) override
            {
            }

        private:
            const bool m_shotgun;
        };
    }
}
