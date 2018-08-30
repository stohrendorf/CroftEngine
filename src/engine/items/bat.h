#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Bat final
        : public AIAgent
{
public:
    Bat(const gsl::not_null<level::Level*>& level,
        const std::string& name,
        const gsl::not_null<const loader::Room*>& room,
        const loader::Item& item,
        const loader::SkeletalModelType& animatedModel)
            : AIAgent( level, name, room, item, animatedModel )
    {
    }

    void update() override;

    void collide(LaraNode& /*other*/, CollisionInfo& /*collisionInfo*/) override
    {
    }
};
}
}
