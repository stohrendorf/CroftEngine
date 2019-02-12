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
                     const gsl::not_null<const loader::file::Room*>& room,
                     const loader::file::Item& item,
                     const loader::file::SkeletalModelType& animatedModel)
            : Switch{level, room, item, animatedModel}
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
