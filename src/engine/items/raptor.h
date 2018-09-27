#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Raptor final
        : public AIAgent
{
public:
    Raptor(const gsl::not_null<level::Level*>& level,
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
