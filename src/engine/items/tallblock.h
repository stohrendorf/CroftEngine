#pragma once

#include "itemnode.h"


namespace engine
{
    namespace items
    {
        class TallBlock final : public ModelItemNode
        {
        public:
            TallBlock(const gsl::not_null<level::Level*>& level,
                      const std::string& name,
                      const gsl::not_null<const loader::Room*>& room,
                      const loader::Item& item,
                      const loader::SkeletalModelType& animatedModel)
                : ModelItemNode(level, name, room, item, true, SaveHitpoints | SaveFlags | NonLot, animatedModel)
            {
                loader::Room::patchHeightsForBlock(*this, -2 * loader::SectorSize);
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void update() override;
        };
    }
}
