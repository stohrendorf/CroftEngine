#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Wolf final
        : public AIAgent
{
public:
    Wolf(const gsl::not_null<level::Level*>& level,
         const std::string& name,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel)
            : AIAgent( level, name, room, item, animatedModel, 384 )
    {
        m_state.health = 6;
        m_state.collidable = true;
        m_state.is_hit = true;
    }

    void update() override;
};
}
}
