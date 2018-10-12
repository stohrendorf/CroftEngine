#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Larson final
        : public AIAgent
{
public:
    Larson(const gsl::not_null<level::Level*>& level,
           const gsl::not_null<const loader::Room*>& room,
           const loader::Item& item,
           const loader::SkeletalModelType& animatedModel);

    void update() override;
};
}
}
