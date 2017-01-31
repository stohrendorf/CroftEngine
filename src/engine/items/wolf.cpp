#include <boost/range/adaptors.hpp>
#include "wolf.h"

#include "engine/heightinfo.h"
#include "level/level.h"
#include "engine/laranode.h"
#include "engine/ai/ai.h"

namespace engine
{
    namespace items
    {
        core::Angle rotateTowardsMoveTarget(ItemNode& npc, const ai::Brain& creatureData, core::Angle maxRotationSpeed)
        {
            if( npc.getHorizontalSpeed() == 0 || maxRotationSpeed == 0_au )
            {
                return 0_au;
            }

            const auto dx = creatureData.moveTarget.X - npc.getPosition().X;
            const auto dz = creatureData.moveTarget.Z - npc.getPosition().Z;
            auto rotation = core::Angle::fromAtan(dx, dz) - npc.getRotation().Y;
            if( rotation > 90_deg || rotation < -90_deg )
            {
                // the move target is behind the NPC
                auto relativeSpeed = npc.getHorizontalSpeed().getCurrentValue() * (1 << 14) / maxRotationSpeed.toAU();
                if( dx * dx + dz * dz < relativeSpeed * relativeSpeed )
                {
                    maxRotationSpeed /= 2;
                }
            }

            if( rotation > maxRotationSpeed )
            {
                rotation = maxRotationSpeed;
            }
            else if( rotation < -maxRotationSpeed )
            {
                rotation = -maxRotationSpeed;
            }

            npc.addYRotation(rotation);
            return rotation;
        }


        bool isPositionOutOfReach(const ItemNode& item, const core::TRCoordinates& testPosition, int currentBoxFloor, const ai::RoutePlanner& lotInfo)
        {
            const auto sectorBoxIdx = item.getLevel().findRealFloorSector(testPosition, item.getCurrentRoom())->boxIndex;
            if( sectorBoxIdx == 0xffff )
            {
                return true;
            }

            const auto& sectorBox = item.getLevel().m_boxes[sectorBoxIdx];
            if( lotInfo.blockMask & sectorBox.overlap_index )
            {
                return true;
            }

            const auto testBoxFloor = currentBoxFloor - sectorBox.floor;
            if( testBoxFloor >= lotInfo.dropHeight && testBoxFloor <= lotInfo.stepHeight )
            {
                // We could step up to/drop down to the test sector.
                return lotInfo.flyHeight != 0 && testPosition.Y > lotInfo.flyHeight + sectorBox.floor;
            }

            return true;
        }


        void rotateCreatureTilt(items::ItemNode& item, core::Angle angle)
        {
            const auto dz = core::Angle(4 * angle.toAU()) - item.getRotation().Z;
            item.addZRotation(util::clamp(dz, -3_deg, +3_deg));
        }


        void rotateCreatureHead(items::Wolf& item, core::Angle angle)
        {
            const auto d = util::clamp(angle - item.m_brain.jointRotation.Y, -5_deg, +5_deg);
            item.m_brain.jointRotation.Y = util::clamp(d + item.m_brain.jointRotation.Y, -90_deg, +90_deg);
        }


        bool anyMovingEnabledItemInReach(const items::Wolf& npc)
        {
            for( const std::shared_ptr<ItemNode>& item : npc.getLevel().m_itemNodes | boost::adaptors::map_values )
            {
                if( !item->m_isActive || item.get() == &npc )
                {
                    continue;
                }

                if( item->m_triggerState == items::TriggerState::Enabled
                    && item->getHorizontalSpeed().getCurrentValue() != 0
                    && item->getPosition().distanceTo(npc.getPosition()) < npc.m_collisionRadius )
                {
                    return true;
                }
            }
            return false;
        }


        bool animateCreature(Wolf& npc, core::Angle rotationToMoveTarget, core::Angle roll)
        {
            //npc.addTime(std::chrono::microseconds(1));
            if( npc.m_triggerState == engine::items::TriggerState::Activated )
            {
                npc.m_health = -16384;
                npc.m_flags2_20_collidable = false;
                //! @todo disposeCreatureData();
                npc.deactivate();
                return false;
            }

            const auto initialPos = npc.getPosition();
            auto bboxTop = std::lround(npc.getBoundingBox().min.y);
            auto npcRoom = npc.getCurrentRoom();
            auto npcSector = npc.getLevel().findRealFloorSector(
                npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, 0),
                &npcRoom);

            if(npcSector->boxIndex == 0xffff)
                return false;

            const auto npcBoxFloor = npc.getLevel().m_boxes[npcSector->boxIndex].floor;

            const auto inSectorX = std::fmod(npc.getPosition().X, loader::SectorSize);
            const auto inSectorZ = std::fmod(npc.getPosition().Z, loader::SectorSize);
            float moveZ = 0;
            float moveX = 0;
            if( inSectorZ < npc.m_collisionRadius )
            {
                // Clamp movement to -Z

                if( isPositionOutOfReach(
                    npc,
                    npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, -npc.m_collisionRadius),
                    npcBoxFloor,
                    npc.m_brain.route) )
                {
                    moveZ = npc.m_collisionRadius - inSectorZ;
                }

                if( inSectorX < npc.m_collisionRadius )
                {
                    // Clamp movement to -X

                    if( isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(-npc.m_collisionRadius, bboxTop, 0),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        moveX = npc.m_collisionRadius - inSectorX;
                    }
                    else if( moveZ == 0
                             && isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(-npc.m_collisionRadius, bboxTop, -npc.m_collisionRadius),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        // -X/-Z must be clamped. Clamp the lateral direction to allow better forward movement.
                        if( npc.getRotation().Y > -135_deg && npc.getRotation().Y < 45_deg )
                        {
                            // We're facing -X/+Z
                            moveZ = npc.m_collisionRadius - inSectorZ;
                        }
                        else
                        {
                            moveX = npc.m_collisionRadius - inSectorX;
                        }
                    }
                }
                else if( inSectorX > loader::SectorSize - npc.m_collisionRadius )
                {
                    // Clamp movement to +X

                    if( isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(npc.m_collisionRadius, bboxTop, 0),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        moveX = loader::SectorSize - npc.m_collisionRadius - inSectorX;
                    }
                    else if( moveZ == 0
                             && isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(npc.m_collisionRadius, bboxTop, -npc.m_collisionRadius),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        // +X/-Z
                        if( npc.getRotation().Y > -45_deg && npc.getRotation().Y < 135_deg )
                        {
                            moveZ = npc.m_collisionRadius - inSectorZ;
                        }
                        else
                        {
                            moveX = loader::SectorSize - npc.m_collisionRadius - inSectorX;
                        }
                    }
                }
            }
            else if( inSectorZ > loader::SectorSize - npc.m_collisionRadius )
            {
                if( isPositionOutOfReach(
                    npc,
                    npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, npc.m_collisionRadius),
                    npcBoxFloor,
                    npc.m_brain.route) )
                {
                    moveZ = loader::SectorSize - npc.m_collisionRadius - inSectorZ;
                }

                if( inSectorX < npc.m_collisionRadius )
                {
                    if( isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(-npc.m_collisionRadius, bboxTop, 0),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        moveX = npc.m_collisionRadius - inSectorX;
                    }
                    else if( moveZ == 0
                             && isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(-npc.m_collisionRadius, bboxTop, -npc.m_collisionRadius),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        if( npc.getRotation().Y > -45_deg && npc.getRotation().Y < 135_deg )
                        {
                            moveX = npc.m_collisionRadius - inSectorX;
                        }
                        else
                        {
                            moveZ = loader::SectorSize - npc.m_collisionRadius - inSectorZ;
                        }
                    }
                }
                else if( inSectorX > loader::SectorSize - npc.m_collisionRadius )
                {
                    if( isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(npc.m_collisionRadius, bboxTop, 0),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        moveX = loader::SectorSize - npc.m_collisionRadius - inSectorX;
                    }
                    else if( moveZ == 0
                             && isPositionOutOfReach(
                        npc,
                        npc.getPosition().toInexact() + core::TRCoordinates(npc.m_collisionRadius, bboxTop, npc.m_collisionRadius),
                        npcBoxFloor,
                        npc.m_brain.route) )
                    {
                        if( npc.getRotation().Y > -135_deg && npc.getRotation().Y < 45_deg )
                        {
                            moveX = loader::SectorSize - npc.m_collisionRadius - inSectorX;
                        }
                        else
                        {
                            moveZ = loader::SectorSize - npc.m_collisionRadius - inSectorZ;
                        }
                    }
                }
            }
            else if( inSectorX < npc.m_collisionRadius )
            {
                if( isPositionOutOfReach(
                    npc,
                    npc.getPosition().toInexact() + core::TRCoordinates(-npc.m_collisionRadius, bboxTop, 0),
                    npcBoxFloor,
                    npc.m_brain.route) )
                {
                    moveX = npc.m_collisionRadius - inSectorX;
                }
            }
            else if(inSectorX > loader::SectorSize - npc.m_collisionRadius)
            {
                if( isPositionOutOfReach(
                    npc,
                    npc.getPosition().toInexact() + core::TRCoordinates(npc.m_collisionRadius, bboxTop, 0),
                    npcBoxFloor,
                    npc.m_brain.route) )
                {
                    moveX = loader::SectorSize - npc.m_collisionRadius - inSectorX;
                }
            }

            npc.moveX(moveX);
            npc.moveZ(moveZ);

            if( moveX != 0 || moveZ != 0 )
            {
                npcSector = npc.getLevel().findRealFloorSector(
                    npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, 0),
                    &npcRoom);
                auto effectiveCurveRoll = util::clamp(core::Angle(8 * roll.toAU()) - npc.getRotation().Z, -3_deg, +3_deg);
                npc.addYRotation(rotationToMoveTarget);
                npc.addZRotation(effectiveCurveRoll);
            }

            if( anyMovingEnabledItemInReach(npc) )
            {
                npc.setPosition(initialPos);
                return true;
            }

            if( npc.m_brain.route.flyHeight != 0 )
            {
                auto dy = util::clamp<float>(npc.m_brain.moveTarget.Y - npc.getPosition().Y,
                                             -npc.m_brain.route.flyHeight,
                                             npc.m_brain.route.flyHeight);
                const auto currentFloor = engine::HeightInfo::fromFloor(npcSector,
                                                                        npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, 0),
                                                                        npc.getLevel().m_cameraController).distance;
                if( dy + npc.getPosition().Y <= currentFloor )
                {
                    const auto currentCeiling = HeightInfo::fromCeiling(npcSector,
                                                                        npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, 0),
                                                                        npc.getLevel().m_cameraController).distance;
                    /** @fixme
                    if( npc is CrocodileInWater )
                    {
                        bboxTop = 0;
                    }
                     */
                    if( npc.getPosition().Y + bboxTop + dy < currentCeiling )
                    {
                        if( npc.getPosition().Y + bboxTop >= currentCeiling )
                        {
                            dy = 0;
                        }
                        else
                        {
                            npc.setX(npc.getPosition().X);
                            npc.setZ(npc.getPosition().Z);
                            dy = npc.m_brain.route.flyHeight;
                        }
                    }
                }
                else if( currentFloor >= npc.getPosition().Y )
                {
                    dy = 0;
                    npc.setY(currentFloor);
                }
                else
                {
                    npc.setX(npc.getPosition().X);
                    npc.setZ(npc.getPosition().Z);
                    dy = -npc.m_brain.route.flyHeight;
                }
                npc.moveY(dy);
                const auto sector = npc.getLevel().findRealFloorSector(
                    npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, 0),
                    &npcRoom);
                npc.setFloorHeight(HeightInfo::fromCeiling(sector,
                                                           npc.getPosition().toInexact() + core::TRCoordinates(0, bboxTop, 0),
                                                           npc.getLevel().m_cameraController).distance);

                core::Angle flyAngle = 0_au;
                if( npc.getHorizontalSpeed().getCurrentValue() != 0 )
                {
                    flyAngle = core::Angle::fromAtan(npc.getHorizontalSpeed().getCurrentValue(), dy);
                }

                npc.setXRotation(util::clamp(flyAngle, -1_deg, +1_deg));

                if( npcRoom != npc.getCurrentRoom() )
                {
                    npc.setCurrentRoom(npcRoom);
                }
                return true;
            }

            if( npc.getPosition().Y - 1 >= npc.getFloorHeight() )
            {
                npc.setY(npc.getFloorHeight());
            }
            else if( npc.getFloorHeight() > 64 + npc.getPosition().Y - 1 )
            {
                npc.moveY(64);
            }

            npc.setXRotation(0_au);
            const auto currentSector = npc.getLevel().findRealFloorSector(
                npc.getPosition().toInexact(),
                &npcRoom);
            npc.setFloorHeight(HeightInfo::fromFloor(currentSector, npc.getPosition().toInexact(), npc.getLevel().m_cameraController).distance);
            if( npcRoom != npc.getCurrentRoom() )
            {
                npc.setCurrentRoom(npcRoom);
            }
            return true;
        }


        void Wolf::updateImpl(const std::chrono::microseconds& /*deltaTime*/, const boost::optional<FrameChangeType>& frameChangeType)
        {
            if(!frameChangeType.is_initialized())
                return;

            if( m_triggerState == TriggerState::Locked )
            {
                m_triggerState = TriggerState::Enabled;
            }

            static constexpr const uint16_t Walking = 1;
            static constexpr const uint16_t Running = 2;
            static constexpr const uint16_t Jumping = 3;
            static constexpr const uint16_t Stalking = 5;
            static constexpr const uint16_t JumpAttack = 6;
            static constexpr const uint16_t Attacking = 7;
            static constexpr const uint16_t LyingDown = 8;
            static constexpr const uint16_t PrepareToStrike = 9;
            static constexpr const uint16_t RunningJump = 10;
            static constexpr const uint16_t Dying = 11;
            static constexpr const uint16_t Biting = 12;

            core::Angle pitch = 0_deg;
            core::Angle roll = 0_deg;
            core::Angle rotationToMoveTarget = 0_deg;
            if( m_health > 0 )
            {
                ai::LookAhead lookAhead(*this, 375);

                if( lookAhead.laraAhead )
                {
                    pitch = lookAhead.pivotAngleToLara;
                }

                m_brain.route.updateMood(m_brain, lookAhead, *this, false, 0x2000);
                rotationToMoveTarget = rotateTowardsMoveTarget(*this, m_brain, m_brain.jointRotation.Z);
                switch( getCurrentState() )
                {
                    case LyingDown:
                        pitch = 0_deg;
                        if( m_brain.mood != ai::Mood::Escape && m_brain.route.getZone(*this) != m_brain.route.getZone(*getLevel().m_lara) )
                        {
                            if( std::rand() % 32768 < 32 )
                            {
                                m_requiredAnimState = Running;
                                setTargetState(Walking);
                            }
                        }
                        else
                        {
                            m_requiredAnimState = PrepareToStrike;
                            setTargetState(Walking);
                        }
                        break;
                    case Walking:
                        if( m_requiredAnimState != 0 )
                        {
                            setTargetState(m_requiredAnimState);
                            m_requiredAnimState = 0;
                        }
                        else
                        {
                            setTargetState(Running);
                        }
                        break;
                    case Running:
                        m_brain.jointRotation.Z = 2_deg;
                        if( m_brain.mood != ai::Mood::Bored )
                        {
                            setTargetState(Stalking);
                            m_requiredAnimState = 0;
                        }
                        else if( std::rand() % 32768 < 32 )
                        {
                            setTargetState(Walking);
                            m_requiredAnimState = LyingDown;
                        }
                        break;
                    case PrepareToStrike:
                        if( m_requiredAnimState != 0 )
                        {
                            setTargetState(m_requiredAnimState);
                            m_requiredAnimState = 0;
                            break;
                        }
                        if( m_brain.mood == ai::Mood::Escape )
                        {
                            setTargetState(Jumping);
                        }
                        else if( lookAhead.pivotDistanceToLaraSq < util::square(345) && lookAhead.enemyFacing )
                        {
                            setTargetState(Biting);
                        }
                        else if( m_brain.mood == ai::Mood::Stalk )
                        {
                            setTargetState(Stalking);
                        }
                        else if( m_brain.mood != ai::Mood::Bored )
                        {
                            setTargetState(Jumping);
                        }
                        else
                        {
                            setTargetState(Walking);
                        }
                        break;
                    case Stalking:
                        m_brain.jointRotation.Z = 2_deg;
                        if( m_brain.mood == ai::Mood::Escape )
                        {
                            setTargetState(Jumping);
                        }
                        else if( lookAhead.pivotDistanceToLaraSq < util::square(345) && lookAhead.enemyFacing )
                        {
                            setTargetState(Biting);
                        }
                        else if( lookAhead.pivotDistanceToLaraSq <= util::square(3*loader::SectorSize) )
                        {
                            if( m_brain.mood == ai::Mood::Attack )
                            {
                                if( !lookAhead.laraAhead
                                    || lookAhead.pivotDistanceToLaraSq > util::square(3*loader::SectorSize/2)
                                    || (lookAhead.laraAngleToPivot < 90_deg && lookAhead.laraAngleToPivot > -90_deg) )
                                {
                                    setTargetState(Jumping);
                                }
                            }
                            else if( std::rand() % 32768 >= 384 )
                            {
                                if( m_brain.mood == ai::Mood::Bored )
                                {
                                    setTargetState(PrepareToStrike);
                                }
                            }
                            else
                            {
                                setTargetState(PrepareToStrike);
                                m_requiredAnimState = Attacking;
                            }
                        }
                        else
                        {
                            setTargetState(Jumping);
                        }
                        break;
                    case Jumping:
                        m_brain.jointRotation.Z = 5_deg;
                        roll = rotationToMoveTarget;
                        if( lookAhead.laraAhead && lookAhead.pivotDistanceToLaraSq < util::square(3*loader::SectorSize/2) )
                        {
                            if( lookAhead.pivotDistanceToLaraSq <= util::square(3*loader::SectorSize/2)/2 || (lookAhead.laraAngleToPivot <= 90_deg && lookAhead.laraAngleToPivot >= -90_deg) )
                            {
                                setTargetState(JumpAttack);
                                m_requiredAnimState = 0;
                            }
                            else
                            {
                                setTargetState(PrepareToStrike);
                                m_requiredAnimState = Stalking;
                            }
                        }
                        else if( m_brain.mood != ai::Mood::Stalk || lookAhead.pivotDistanceToLaraSq >= util::square(3*loader::SectorSize) )
                        {
                            if( m_brain.mood == ai::Mood::Bored )
                            {
                                setTargetState(PrepareToStrike);
                            }
                        }
                        else
                        {
                            setTargetState(PrepareToStrike);
                            m_requiredAnimState = Stalking;
                        }
                        break;
                    case JumpAttack:
                        roll = rotationToMoveTarget;
                        if( m_requiredAnimState == 0 /** @fixme && this->touch_bits & 0x774F */ )
                        {
                            //! @todo show blood splatter fx
                            getLevel().m_lara->m_flags2_10_isHit = true;
                            getLevel().m_lara->setHealth(getLevel().m_lara->getHealth() - 50);
                            m_requiredAnimState = Jumping;
                        }
                        setTargetState(Jumping);
                        break;
                    case Biting:
                        if( m_requiredAnimState == 0 /** @fixme && this->touch_bits & 0x774F */ && lookAhead.laraAhead )
                        {
                            //! @todo show blood splatter fx
                            getLevel().m_lara->m_flags2_10_isHit = true;
                            getLevel().m_lara->setHealth(getLevel().m_lara->getHealth() - 100);
                            m_requiredAnimState = PrepareToStrike;
                        }
                        break;
                    default:
                        break;
                }
            }
            else if( getCurrentState() != Dying )
            {
                setAnimIdGlobal(getLevel().m_animatedModels[7]->animationIndex + 20 + 3 * std::rand() / RAND_MAX, 0);
            }
            rotateCreatureTilt(*this, roll);
            rotateCreatureHead(*this, pitch);
            animateCreature(*this, rotationToMoveTarget, roll);
        }
    }
}
