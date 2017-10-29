#include "wolf.h"

#include "engine/heightinfo.h"
#include "level/level.h"
#include "engine/laranode.h"
#include "engine/ai/ai.h"

#include <boost/range/adaptors.hpp>

namespace engine
{
    namespace items
    {
        void Wolf::update()
        {
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
            if( getHealth() > 0 )
            {
                ai::LookAhead lookAhead(*this, 375);

                if( lookAhead.laraAhead )
                {
                    pitch = lookAhead.pivotAngleToLara;
                }

                getBrain().route.updateMood(getBrain(), lookAhead, *this, false, 0x2000);
                rotationToMoveTarget = rotateTowardsMoveTarget(getBrain(), getBrain().jointRotation.Z);
                switch( getCurrentState() )
                {
                    case LyingDown:
                        pitch = 0_deg;
                        if( getBrain().mood != ai::Mood::Escape && getZone() != getBrain().route.getZone(*getLevel().m_lara) )
                        {
                            if( std::rand() % 32768 < 32 )
                            {
                                m_requiredAnimState = Running;
                                m_state.goal_anim_state = Walking;
                            }
                        }
                        else
                        {
                            m_requiredAnimState = PrepareToStrike;
                            m_state.goal_anim_state = Walking;
                        }
                        break;
                    case Walking:
                        if( m_requiredAnimState != 0 )
                        {
                            m_state.goal_anim_state = m_requiredAnimState;
                            m_requiredAnimState = 0;
                        }
                        else
                        {
                            m_state.goal_anim_state = Running;
                        }
                        break;
                    case Running:
                        getBrain().jointRotation.Z = 2_deg;
                        if( getBrain().mood != ai::Mood::Bored )
                        {
                            m_state.goal_anim_state = Stalking;
                            m_requiredAnimState = 0;
                        }
                        else if( std::rand() % 32768 < 32 )
                        {
                            m_state.goal_anim_state = Walking;
                            m_requiredAnimState = LyingDown;
                        }
                        break;
                    case PrepareToStrike:
                        if( m_requiredAnimState != 0 )
                        {
                            m_state.goal_anim_state = m_requiredAnimState;
                            m_requiredAnimState = 0;
                            break;
                        }
                        if( getBrain().mood == ai::Mood::Escape )
                        {
                            m_state.goal_anim_state = Jumping;
                        }
                        else if( lookAhead.pivotDistanceToLaraSq < util::square(345) && lookAhead.enemyFacing )
                        {
                            m_state.goal_anim_state = Biting;
                        }
                        else if( getBrain().mood == ai::Mood::Stalk )
                        {
                            m_state.goal_anim_state = Stalking;
                        }
                        else if( getBrain().mood != ai::Mood::Bored )
                        {
                            m_state.goal_anim_state = Jumping;
                        }
                        else
                        {
                            m_state.goal_anim_state = Walking;
                        }
                        break;
                    case Stalking:
                        getBrain().jointRotation.Z = 2_deg;
                        if( getBrain().mood == ai::Mood::Escape )
                        {
                            m_state.goal_anim_state = Jumping;
                        }
                        else if( lookAhead.pivotDistanceToLaraSq < util::square(345) && lookAhead.enemyFacing )
                        {
                            m_state.goal_anim_state = Biting;
                        }
                        else if( lookAhead.pivotDistanceToLaraSq <= util::square(3*loader::SectorSize) )
                        {
                            if( getBrain().mood == ai::Mood::Attack )
                            {
                                if( !lookAhead.laraAhead
                                    || lookAhead.pivotDistanceToLaraSq > util::square(3*loader::SectorSize/2)
                                    || (lookAhead.laraAngleToPivot < 90_deg && lookAhead.laraAngleToPivot > -90_deg) )
                                {
                                    m_state.goal_anim_state = Jumping;
                                }
                            }
                            else if( std::rand() % 32768 >= 384 )
                            {
                                if( getBrain().mood == ai::Mood::Bored )
                                {
                                    m_state.goal_anim_state = PrepareToStrike;
                                }
                            }
                            else
                            {
                                m_state.goal_anim_state = PrepareToStrike;
                                m_requiredAnimState = Attacking;
                            }
                        }
                        else
                        {
                            m_state.goal_anim_state = Jumping;
                        }
                        break;
                    case Jumping:
                        getBrain().jointRotation.Z = 5_deg;
                        roll = rotationToMoveTarget;
                        if( lookAhead.laraAhead && lookAhead.pivotDistanceToLaraSq < util::square(3*loader::SectorSize/2) )
                        {
                            if( lookAhead.pivotDistanceToLaraSq <= util::square(3*loader::SectorSize/2)/2 || (lookAhead.laraAngleToPivot <= 90_deg && lookAhead.laraAngleToPivot >= -90_deg) )
                            {
                                m_state.goal_anim_state = JumpAttack;
                                m_requiredAnimState = 0;
                            }
                            else
                            {
                                m_state.goal_anim_state = PrepareToStrike;
                                m_requiredAnimState = Stalking;
                            }
                        }
                        else if( getBrain().mood != ai::Mood::Stalk || lookAhead.pivotDistanceToLaraSq >= util::square(3*loader::SectorSize) )
                        {
                            if( getBrain().mood == ai::Mood::Bored )
                            {
                                m_state.goal_anim_state = PrepareToStrike;
                            }
                        }
                        else
                        {
                            m_state.goal_anim_state = PrepareToStrike;
                            m_requiredAnimState = Stalking;
                        }
                        break;
                    case JumpAttack:
                        roll = rotationToMoveTarget;
                        if( m_requiredAnimState == 0 /** @fixme && this->touch_bits & 0x774F */ )
                        {
                            //! @todo show blood splatter fx
                            getLevel().m_lara->m_state.is_hit = true;
                            getLevel().m_lara->m_state.health -= 50;
                            m_requiredAnimState = Jumping;
                        }
                        m_state.goal_anim_state = Jumping;
                        break;
                    case Biting:
                        if( m_requiredAnimState == 0 /** @fixme && this->touch_bits & 0x774F */ && lookAhead.laraAhead )
                        {
                            //! @todo show blood splatter fx
                            getLevel().m_lara->m_state.is_hit = true;
                            getLevel().m_lara->m_state.health -= 100;
                            m_requiredAnimState = PrepareToStrike;
                        }
                        break;
                    default:
                        break;
                }
            }
            else if( getCurrentState() != Dying )
            {
                getSkeleton()->setAnimIdGlobal(m_state, getLevel().m_animatedModels[7]->animationIndex + 20 + 3 * std::rand() / RAND_MAX, 0);
            }
            rotateCreatureTilt(roll);
            rotateCreatureHead(pitch);
            animateCreature(rotationToMoveTarget, roll);
        }
    }
}
