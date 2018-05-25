#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class CollapsibleFloor final : public ModelItemNode
{
public:
    CollapsibleFloor(const gsl::not_null<level::Level*>& level,
                     const std::string& name,
                     const gsl::not_null<const loader::Room*>& room,
                     const loader::Item& item,
                     const loader::SkeletalModelType& animatedModel)
            : ModelItemNode( level, name, room, item, true, animatedModel )
    {
        m_state.is_hit = true;
        m_state.collidable = true;
    }

    void update() override;

    void collide(LaraNode& /*other*/, CollisionInfo& /*collisionInfo*/) override
    {
    }

    void patchFloor(const core::TRCoordinates& pos, int& y) override
    {
        if( pos.Y > m_state.position.position.Y - 512 )
            return;

        if( m_state.current_anim_state != 0 && m_state.current_anim_state != 1 )
            return;

        y = m_state.position.position.Y - 512;
    }

    void patchCeiling(const core::TRCoordinates& pos, int& y) override
    {
        if( pos.Y <= m_state.position.position.Y - 512 )
            return;

        if( m_state.current_anim_state != 0 && m_state.current_anim_state != 1 )
            return;

        y = m_state.position.position.Y - 256;
    }
};
}
}
