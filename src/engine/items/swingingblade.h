#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class SwingingBlade final : public ModelItemNode
        {
        public:
            SwingingBlade(const gsl::not_null<level::Level*>& level,
                          const std::string& name,
                          const gsl::not_null<const loader::Room*>& room,
                          const loader::Item& item,
                          const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, item, true, SaveHitpoints | SaveFlags, animatedModel)
            {
            }


            void update() override;


            void collide(LaraNode& /*other*/, CollisionInfo& /*collisionInfo*/) override
            {
            }
        };
    }
}
