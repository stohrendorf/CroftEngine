#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class Item_41_TrapDoorUp final : public ItemNode
        {
        public:
            Item_41_TrapDoorUp(const gsl::not_null<level::Level*>& level,
                               const std::string& name,
                               const gsl::not_null<const loader::Room*>& room,
                               const gsl::not_null<loader::Item*>& item,
                               const loader::AnimatedModel& animatedModel)
                    : ItemNode( level, name, room, item, true, 0x30, animatedModel )
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/) override
            {
            }


            void onInteract(LaraNode& /*lara*/) override
            {
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;


            void patchFloor(const core::TRCoordinates& pos, long& y) override
            {
                if( getCurrentState() != 1 || !possiblyOnTrapdoor( pos ) || pos.Y > getPosition().Y )
                    return;

                y = std::lround( getPosition().Y );
            }


            void patchCeiling(const core::TRCoordinates& pos, long& y) override
            {
                if( getCurrentState() != 1 || !possiblyOnTrapdoor( pos ) || pos.Y <= getPosition().Y )
                    return;

                y = std::lround( getPosition().Y + loader::QuarterSectorSize );
            }


        private:
            bool possiblyOnTrapdoor(const core::TRCoordinates& pos) const
            {
                auto sx = std::lround( std::floor( getPosition().X / loader::SectorSize ) );
                auto sz = std::lround( std::floor( getPosition().Z / loader::SectorSize ) );
                auto psx = pos.X / loader::SectorSize;
                auto psz = pos.Z / loader::SectorSize;
                auto axis = core::axisFromAngle( getRotation().Y, 0_au );
                BOOST_ASSERT( axis.is_initialized() );
                if( *axis == core::Axis::PosZ && sx == psx && sz - 1 == psz && sz - 2 == psz )
                {
                    return true;
                }
                if( *axis == core::Axis::NegZ && sx == psx && sz + 1 == psz && sz + 2 == psz )
                {
                    return true;
                }
                if( *axis == core::Axis::PosX && sz == psz && sx - 1 == psx && sx - 2 == psx )
                {
                    return true;
                }
                if( *axis != core::Axis::NegX || sz != psz || sx + 1 != psx || sx + 2 != psx )
                {
                    return false;
                }
                return true;
            }
        };
    }
}
