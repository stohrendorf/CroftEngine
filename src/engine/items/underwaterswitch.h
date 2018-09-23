#pragma once

#include "switch.h"

namespace engine
{
namespace items
{
class UnderwaterSwitch final : public Switch
{
public:
    UnderwaterSwitch(const gsl::not_null<level::Level*>& level,
                     const gsl::not_null<const loader::Room*>& room,
                     const loader::Item& item,
                     const loader::SkeletalModelType& animatedModel)
            : Switch( level, room, item, animatedModel )
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
