#pragma once

#include "aiagent.h"

#include "engine/ai/ai.h"

namespace engine
{
namespace items
{
class Crocodile final
        : public AIAgent
{
public:
    Crocodile(const gsl::not_null<level::Level*>& level,
              const gsl::not_null<const loader::Room*>& room,
              const loader::Item& item,
              const loader::SkeletalModelType& animatedModel)
            : AIAgent{level, room, item, animatedModel}
    {
    }

    void update() override;

    void load(const YAML::Node& n) override
    {
        auto newType = EnumUtil<TR1ItemId>::fromString( n["state"]["type"].as<std::string>() );

        if( newType == TR1ItemId::CrocodileOnLand || newType == TR1ItemId::CrocodileInWater )
            m_state.type = newType;

        AIAgent::load( n );
    }
};
}
}
