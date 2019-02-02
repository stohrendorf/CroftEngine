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
              const gsl::not_null<const loader::Room*>& room,
              const loader::Item& item,
              const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
        loader::Room::patchHeightsForBlock( *this, -2 * loader::SectorSize );
    }

    void update() override;

    void load(const YAML::Node& n) override;
};
}
}
