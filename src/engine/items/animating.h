#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Animating final : public ModelItemNode
{
public:
    Animating(const gsl::not_null<loader::file::level::Level*>& level,
              const gsl::not_null<const loader::file::Room*>& room,
              const loader::file::Item& item,
              const loader::file::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
    }

    void update() override;
};
}
}
