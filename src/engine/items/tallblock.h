#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class TallBlock final : public ModelItemNode
{
public:
    TallBlock(const gsl::not_null<Engine*>& engine,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{ engine, room, item, true, animatedModel }
    {
        loader::file::Room::patchHeightsForBlock( *this, -2 * core::SectorSize );
    }

    void update() override;

    void load(const YAML::Node& n) override;
};
}
}
