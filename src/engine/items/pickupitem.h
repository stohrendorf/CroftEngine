#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class PickupItem final : public ModelItemNode
        {
        public:
            PickupItem(const gsl::not_null<level::Level*>& level,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       const loader::Item& item,
                       const loader::SkeletalModelType& animatedModel,
                       bool shotgun = false)
                : ModelItemNode(level, name, room, item, true, SaveHitpoints | SaveFlags, animatedModel)
                , m_shotgun{shotgun}
            {
            }


            void update() override
            {
            }


            void collide(LaraNode& other, CollisionInfo& collisionInfo) override;


        private:
            const bool m_shotgun;
        };
    }
}
