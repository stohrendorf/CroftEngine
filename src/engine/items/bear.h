#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Bear final
        : public AIAgent
{
public:
    Bear(const gsl::not_null<level::Level*>& level,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel)
            : AIAgent{level, room, item, animatedModel}
    {
    }

    void update() override;
};
}
}
