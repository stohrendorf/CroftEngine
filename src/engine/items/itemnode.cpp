#include "itemnode.h"

#include "engine/laranode.h"
#include "level/level.h"


namespace engine
{
    namespace items
    {
        void ItemNode::applyTransform()
        {
            glm::vec3 tr;

            if( auto parent = m_position.room )
            {
                tr = m_position.position.toRenderSystem() - parent->position.toRenderSystem();
            }
            else
            {
                tr = m_position.position.toRenderSystem();
            }

            setLocalMatrix(glm::translate(glm::mat4{1.0f}, tr) * getRotation().toMatrix());

            updateSounds();
        }


        ItemNode::ItemNode(const gsl::not_null<level::Level*>& level,
                           const std::string& name,
                           const gsl::not_null<const loader::Room*>& room,
                           const core::Angle& angle,
                           const core::TRCoordinates& position,
                           const floordata::ActivationState& activationState,
                           bool hasProcessAnimCommandsOverride,
                           Characteristics characteristics,
                           int16_t darkness,
                           const loader::AnimatedModel& animatedModel)
            : SkeletalModelNode(name, level, animatedModel)
            , m_position(room, position)
            , m_rotation(0_deg, angle, 0_deg)
            , m_level(level)
            , m_activationState(activationState)
            , m_hasProcessAnimCommandsOverride(hasProcessAnimCommandsOverride)
            , m_characteristics(characteristics)
            , m_darkness{darkness}
        {
            BOOST_ASSERT(room->isInnerPositionXZ(position));

            if( m_activationState.isOneshot() )
            {
                setEnabled(false);
            }

            if( m_activationState.isOneshot() )
            {
                m_activationState.setOneshot(false);
                m_triggerState = TriggerState::Locked;
            }

            if( m_activationState.isFullyActivated() )
            {
                m_activationState.fullyDeactivate();
                m_activationState.setInverted(true);
                activate();
                m_triggerState = TriggerState::Enabled;
            }
        }


        void ItemNode::setCurrentRoom(const loader::Room* newRoom)
        {
            if( newRoom == m_position.room )
            {
                return;
            }

            if( newRoom == nullptr )
            {
                BOOST_LOG_TRIVIAL(fatal) << "No room to switch to.";
                return;
            }
            BOOST_LOG_TRIVIAL(debug) << "Room switch of " << getId() << " to " << newRoom->node->getId();

            newRoom->node->addChild(shared_from_this());

            m_position.room = newRoom;
        }


        void ItemNode::update()
        {
            const auto endOfAnim = advanceFrame();

            m_flags2_10_isHit = false;

            if( endOfAnim )
            {
                const loader::Animation& animation = getCurrentAnimData();
                BOOST_ASSERT(animation.animCommandCount == 0 || animation.animCommandIndex < getLevel().m_animCommands.size());
                const auto* cmd = animation.animCommandCount == 0 ? nullptr : &getLevel().m_animCommands[animation.animCommandIndex];
                for( uint16_t i = 0; i < animation.animCommandCount; ++i )
                {
                    BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                    const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                    ++cmd;
                    switch( opcode )
                    {
                        case AnimCommandOpcode::SetPosition:
                            moveLocal(
                                cmd[0],
                                cmd[1],
                                cmd[2]
                            );
                            cmd += 3;
                            break;
                        case AnimCommandOpcode::StartFalling:
                            m_fallSpeed = cmd[0];
                            m_horizontalSpeed = cmd[1];
                            m_falling = true;
                            cmd += 2;
                            break;
                        case AnimCommandOpcode::PlaySound:
                            cmd += 2;
                            break;
                        case AnimCommandOpcode::PlayEffect:
                            cmd += 2;
                            break;
                        case AnimCommandOpcode::Kill:
                            m_triggerState = TriggerState::Activated;
                            break;
                        default:
                            break;
                    }
                }

                const loader::Animation& currentAnim = getCurrentAnimData();
                setAnimIdGlobal(currentAnim.nextAnimation, currentAnim.nextFrame);
                setTargetState(getCurrentState());
            }

            const loader::Animation& animation = getCurrentAnimData();
            BOOST_ASSERT(animation.animCommandCount == 0 || animation.animCommandIndex < getLevel().m_animCommands.size());
            const auto* cmd = animation.animCommandCount == 0 ? nullptr : &getLevel().m_animCommands[animation.animCommandIndex];
            for( uint16_t i = 0; i < animation.animCommandCount; ++i )
            {
                BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch( opcode )
                {
                    case AnimCommandOpcode::SetPosition:
                        cmd += 3;
                        break;
                    case AnimCommandOpcode::StartFalling:
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlaySound:
                        if( getCurrentFrame() == cmd[0] )
                        {
                            playSoundEffect(cmd[1]);
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if( getCurrentFrame() == cmd[0] )
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                            if( cmd[1] == 0 )
                            {
                                addYRotation(180_deg);
                            }
                            else if( cmd[1] == 12 )
                            {
                                getLevel().m_lara->setHandStatus(0);
                            }
                            //! @todo Execute anim effect cmd[1]
                        }
                        cmd += 2;
                        break;
                    default:
                        break;
                }
            }

            applyMovement(false);
        }


        void ItemNode::activate()
        {
            if( !m_hasProcessAnimCommandsOverride )
            {
                m_triggerState = TriggerState::Disabled;
                return;
            }

            if( m_isActive )
            {
                //BOOST_LOG_TRIVIAL(warning) << "Item controller " << getId() << " already active";
            }
            else
            {
                BOOST_LOG_TRIVIAL(trace) << "Activating item controller " << getId();
            }

            m_isActive = true;
        }


        void ItemNode::deactivate()
        {
            if( !m_isActive )
            {
                //BOOST_LOG_TRIVIAL(warning) << "Item controller " << getId() << " already inactive";
            }
            else
            {
                BOOST_LOG_TRIVIAL(trace) << "Deactivating item controller " << getId();
            }

            m_isActive = false;
        }


        std::shared_ptr<audio::SourceHandle> ItemNode::playSoundEffect(int id)
        {
            auto handle = getLevel().playSound(id, getTranslationWorld());
            if( handle != nullptr )
            {
                m_sounds.insert(handle);
            }
            return handle;
        }


        bool ItemNode::triggerKey()
        {
            if( getLevel().m_lara->getHandStatus() != 0 )
            {
                return false;
            }

            if( m_triggerState != TriggerState::Enabled )
            {
                return false;
            }

            m_triggerState = TriggerState::Activated;
            return true;
        }


        void ItemNode::updateSounds()
        {
            decltype(m_sounds) cleaned;
            std::copy_if(m_sounds.begin(), m_sounds.end(), inserter(cleaned, cleaned.end()),
                         [](const std::weak_ptr<audio::SourceHandle>& h)
                     {
                         return h.expired();
                     });

            m_sounds = std::move(cleaned);

            for( const std::weak_ptr<audio::SourceHandle>& handle : m_sounds )
            {
                std::shared_ptr<audio::SourceHandle> lockedHandle = handle.lock();
                lockedHandle->setPosition(getTranslationWorld());
            }
        }


        bool InteractionLimits::canInteract(const ItemNode& item, const LaraNode& lara) const
        {
            const auto angle = lara.getRotation() - item.getRotation();
            if( angle.X < minAngle.X || angle.X > maxAngle.X
                || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
                || angle.Z < minAngle.Z || angle.Z > maxAngle.Z )
            {
                return false;
            }

            auto dist = glm::vec4(lara.getPosition().toRenderSystem() - item.getPosition().toRenderSystem(), 1.0f);
            glm::vec3 tdist{dist * item.getRotation().toMatrix()};

            return distance.contains(tdist, 1);
        }


        void ItemNode::applyMovement(bool forLara)
        {
            if( m_falling )
            {
                if( getFallSpeed() >= 128 )
                {
                    m_fallSpeed += 1;
                }
                else
                {
                    m_fallSpeed += 6;
                }

                if( forLara )
                {
                    // we only add accelleration here
                    m_horizontalSpeed += calculateFloorSpeed(0) - calculateFloorSpeed(-1);
                }
            }
            else
            {
                m_horizontalSpeed = calculateFloorSpeed();
            }

            move(
                getMovementAngle().sin() * m_horizontalSpeed,
                m_falling ? m_fallSpeed : 0,
                getMovementAngle().cos() * m_horizontalSpeed
            );

            applyTransform();

            updatePose();
            updateLighting();
        }


        boost::optional<uint16_t> ItemNode::getCurrentBox() const
        {
            auto sector = m_position.room->getInnerSectorByAbsolutePosition(m_position.position);
            if( sector->boxIndex == 0xffff )
            {
                BOOST_LOG_TRIVIAL(warning) << "Not within a box: " << getId();
                return {};
            }

            return sector->boxIndex;
        }
    }
}
