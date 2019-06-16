#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class BridgeFlat final : public ModelItemNode
{
public:
    BridgeFlat(const gsl::not_null<engine::Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const loader::file::SkeletalModelType& animatedModel)
        : ModelItemNode{engine, room, item, false, animatedModel}
    {
    }

    void patchFloor(const core::TRVec& pos, core::Length& y) override
    {
        if(pos.Y <= m_state.position.position.Y)
            y = m_state.position.position.Y;
    }

    void patchCeiling(const core::TRVec& pos, core::Length& y) override
    {
        if(pos.Y <= m_state.position.position.Y)
            return;

        y = m_state.position.position.Y + core::QuarterSectorSize;
    }
};
} // namespace items
} // namespace engine
