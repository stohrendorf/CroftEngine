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
    Raptor(const gsl::not_null<loader::file::level::Level*>& level,
           const gsl::not_null<const loader::file::Room*>& room,
           const loader::file::Item& item,
           const loader::file::SkeletalModelType& animatedModel)
            : AIAgent{level, room, item, animatedModel}
    {
    }

    void update() override;
};
}
}
