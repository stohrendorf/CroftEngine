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
                     const gsl::not_null<const loader::Room*>& room,
                     const loader::Item& item,
                     const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
    }

    void update() override;

    void patchFloor(const core::TRVec& pos, core::Length& y) override
    {
        if( pos.Y > m_state.position.position.Y - 512_len )
            return;

        if( m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as )
            return;

        y = m_state.position.position.Y - 512_len;
    }

    void patchCeiling(const core::TRVec& pos, core::Length& y) override
    {
        if( pos.Y <= m_state.position.position.Y - 512_len )
            return;

        if( m_state.current_anim_state != 0_as && m_state.current_anim_state != 1_as )
            return;

        y = m_state.position.position.Y - 256_len;
    }
};
}
}
