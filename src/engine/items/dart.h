#pragma once

#include "itemnode.h"
#include "engine/particle.h"

namespace engine
{
namespace items
{
class Dart final : public ModelItemNode
{
public:
    Dart(const gsl::not_null<level::Level*>& level,
         const std::string& name,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, true, animatedModel )
    {
        m_state.collidable = true;
    }

    void collide(LaraNode& lara, CollisionInfo& info) override;

    void update() override;
};
}
}
