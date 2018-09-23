#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class BridgeFlat final : public ModelItemNode
{
public:
    BridgeFlat(const gsl::not_null<level::Level*>& level,
               const gsl::not_null<const loader::Room*>& room,
               const loader::Item& item,
               const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, false, animatedModel}
    {
    }

    void patchFloor(const core::TRVec& pos, int& y) override
    {
        if( pos.Y <= m_state.position.position.Y )
            y = m_state.position.position.Y;
    }

    void patchCeiling(const core::TRVec& pos, int& y) override
    {
        if( pos.Y <= m_state.position.position.Y )
            return;

        y = m_state.position.position.Y + loader::QuarterSectorSize;
    }
};
}
}
