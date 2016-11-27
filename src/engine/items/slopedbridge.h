#pragma once

#include "itemnode.h"

namespace engine
{
    namespace items
    {
        class SlopedBridge : public ItemNode
        {
        private:
            const int m_div;
        public:
            SlopedBridge(const gsl::not_null<level::Level*>& level,
                              const std::string& name,
                              const gsl::not_null<const loader::Room*>& room,
                              const gsl::not_null<loader::Item*>& item,
                              const loader::AnimatedModel& animatedModel,
                              int div)
                    : ItemNode( level, name, room, item, false, 0, animatedModel )
                    , m_div( div )
            {
            }


            void updateImpl(const std::chrono::microseconds& /*deltaTime*/) override final
            {
            }


            void onInteract(LaraNode& /*lara*/) override final
            {
            }


            void onFrameChanged(FrameChangeType /*frameChangeType*/) override
            {
            }


            void patchFloor(const core::TRCoordinates& pos, long& y) override final
            {
                auto tmp = std::lround( getPosition().Y + getBridgeSlopeHeight( pos ) / m_div );
                if( pos.Y <= tmp )
                    y = tmp;
            }


            void patchCeiling(const core::TRCoordinates& pos, long& y) override final
            {
                auto tmp = std::lround( getPosition().Y + getBridgeSlopeHeight( pos ) / m_div );
                if( pos.Y <= tmp )
                    return;

                y = tmp + loader::QuarterSectorSize;
            }


        private:
            long getBridgeSlopeHeight(const core::TRCoordinates& pos) const
            {
                auto axis = core::axisFromAngle( getRotation().Y, 1_deg );
                Expects( axis.is_initialized() );

                switch( *axis )
                {
                    case core::Axis::PosZ:return loader::SectorSize - 1 - pos.X % loader::SectorSize;
                    case core::Axis::PosX:return pos.Z % loader::SectorSize;
                    case core::Axis::NegZ:return pos.X % loader::SectorSize;
                    case core::Axis::NegX:return loader::SectorSize - 1 - pos.Z % loader::SectorSize;
                    default:return 0;
                }
            }
        };


        class BridgeSlope1 final : public SlopedBridge
        {
        public:
            BridgeSlope1(const gsl::not_null<level::Level*>& level,
                                 const std::string& name,
                                 const gsl::not_null<const loader::Room*>& room,
                                 const gsl::not_null<loader::Item*>& item,
                                 const loader::AnimatedModel& animatedModel)
                    : SlopedBridge( level, name, room, item, animatedModel, 4 )
            {
            }
        };


        class BridgeSlope2 final : public SlopedBridge
        {
        public:
            BridgeSlope2(const gsl::not_null<level::Level*>& level,
                                 const std::string& name,
                                 const gsl::not_null<const loader::Room*>& room,
                                 const gsl::not_null<loader::Item*>& item,
                                 const loader::AnimatedModel& animatedModel)
                    : SlopedBridge( level, name, room, item, animatedModel, 2 )
            {
            }
        };
    }
}