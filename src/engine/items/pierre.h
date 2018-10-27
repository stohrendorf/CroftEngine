#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Pierre final
        : public AIAgent
{
public:
    Pierre(const gsl::not_null<level::Level*>& level,
           const gsl::not_null<const loader::Room*>& room,
           const loader::Item& item,
           const loader::SkeletalModelType& animatedModel);

    void update() override;
};
}
}
