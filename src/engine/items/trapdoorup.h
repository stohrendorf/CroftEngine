#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class TrapDoorUp final : public ModelItemNode
{
public:
    TrapDoorUp(const gsl::not_null<level::Level*>& level,
               const gsl::not_null<const loader::Room*>& room,
               const loader::Item& item,
               const loader::SkeletalModelType& animatedModel)
            : ModelItemNode{level, room, item, true, animatedModel}
    {
    }

    void update() override;

    void patchFloor(const core::TRVec& pos, int& y) override
    {
        if( m_state.current_anim_state != 1_as || !possiblyOnTrapdoor( pos ) || pos.Y > m_state.position.position.Y )
            return;

        y = m_state.position.position.Y;
    }

    void patchCeiling(const core::TRVec& pos, int& y) override
    {
        if( m_state.current_anim_state != 1_as || !possiblyOnTrapdoor( pos ) || pos.Y <= m_state.position.position.Y )
            return;

        y = m_state.position.position.Y + loader::QuarterSectorSize;
    }

private:
    bool possiblyOnTrapdoor(const core::TRVec& pos) const
    {
        const auto trapdoorSectorX = m_state.position.position.X / loader::SectorSize;
        const auto trapdoorSectorZ = m_state.position.position.Z / loader::SectorSize;
        const auto posSectorX = pos.X / loader::SectorSize;
        const auto posSectorZ = pos.Z / loader::SectorSize;
        auto trapdoorAxis = axisFromAngle( m_state.rotation.Y, 1_au );
        BOOST_ASSERT( trapdoorAxis.is_initialized() );

        if( *trapdoorAxis == core::Axis::PosZ && trapdoorSectorX == posSectorX
            && (trapdoorSectorZ - 1 == posSectorZ || trapdoorSectorZ - 2 == posSectorZ) )
            return true;
        if( *trapdoorAxis == core::Axis::NegZ && trapdoorSectorX == posSectorX
            && (trapdoorSectorZ + 1 == posSectorZ || trapdoorSectorZ + 2 == posSectorZ) )
            return true;
        if( *trapdoorAxis == core::Axis::PosX && trapdoorSectorZ == posSectorZ
            && (trapdoorSectorX - 1 == posSectorX || trapdoorSectorX - 2 == posSectorX) )
            return true;
        if( *trapdoorAxis == core::Axis::NegX && trapdoorSectorZ == posSectorZ
            && (trapdoorSectorX + 1 == posSectorX || trapdoorSectorX + 2 == posSectorX) )
            return true;

        return false;
    }
};
}
}
