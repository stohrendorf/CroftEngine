#pragma once

#include "switch.h"

namespace engine
{
namespace items
{
class UnderwaterSwitch final : public Switch
{
public:
    UnderwaterSwitch(const gsl::not_null<Engine*>& engine,
                     const gsl::not_null<const loader::file::Room*>& room,
                     const loader::file::Item& item,
                     const loader::file::SkeletalModelType& animatedModel)
        : Switch{ engine, room, item, animatedModel }
    {
    }

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;
};
}
}
