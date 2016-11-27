#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class StubItem final : public ItemNode
        {
        public:
            StubItem(const gsl::not_null<level::Level*>& level,
                     const std::string& name,
                     const gsl::not_null<const loader::Room*>& room,
                     const gsl::not_null<loader::Item*>& item,
                     const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, false, 0, animatedModel )
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/) override
            {
            }
        };
    }
}
