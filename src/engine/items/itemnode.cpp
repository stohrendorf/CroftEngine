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
                           const core::ExactTRCoordinates& position,
                           const floordata::ActivationState& activationState,
                           bool hasProcessAnimCommandsOverride,
                           uint8_t characteristics,
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
            BOOST_ASSERT(room->isInnerPositionXZ(position.toInexact()));

            if( m_activationState.isOneshot() )
            {
                setEnabled(false);
            }

            if( m_activationState.isOneshot() )
            {
                m_activationState.setOneshot(false);
                m_triggerState = engine::items::TriggerState::Locked;
            }

            if( m_activationState.isFullyActivated() )
            {
                m_activationState.fullyDeactivate();
                m_activationState.setInverted(true);
                activate();
                m_triggerState = engine::items::TriggerState::Enabled;
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


        void ItemNode::onFrameChanged(FrameChangeType frameChangeType)
        {
            m_flags2_10_isHit = false;

            const loader::Animation& animation = getLevel().m_animations[getAnimId()];
            if( animation.animCommandCount <= 0 )
            {
                return;
            }

            BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
            const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
            for( uint16_t i = 0; i < animation.animCommandCount; ++i )
            {
                BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch( opcode )
                {
                    case AnimCommandOpcode::SetPosition:
                        if( frameChangeType == FrameChangeType::EndOfAnim )
                        {
                            moveLocal(
                                cmd[0],
                                cmd[1],
                                cmd[2]
                                     );
                        }
                        cmd += 3;
                        break;
                    case AnimCommandOpcode::SetVelocity:
                        if( frameChangeType == FrameChangeType::EndOfAnim )
                        {
                            m_fallSpeed = cmd[0];
                            m_falling = true;
                            m_horizontalSpeed = cmd[1];
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::EmptyHands:
                        break;
                    case AnimCommandOpcode::PlaySound:
                        if( frameChangeType == FrameChangeType::NewFrame
                            && core::toFrame(getCurrentTime()) == cmd[0] )
                        {
                            playSoundEffect(cmd[1]);
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if( core::toFrame(getCurrentTime()) == cmd[0] )
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                            if( frameChangeType == FrameChangeType::NewFrame && cmd[1] == 0 )
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
                    case AnimCommandOpcode::Kill:
                        if( frameChangeType == FrameChangeType::EndOfAnim )
                        {
                            m_triggerState = engine::items::TriggerState::Activated;
                        }
                        break;
                    default:
                        break;
                }
            }
        }


        void ItemNode::activate()
        {
            if( !m_hasProcessAnimCommandsOverride )
            {
                m_triggerState = engine::items::TriggerState::Disabled;
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

            if( m_triggerState != engine::items::TriggerState::Enabled )
            {
                return false;
            }

            m_triggerState = engine::items::TriggerState::Activated;
            return true;
        }


        void ItemNode::updateSounds()
        {
            decltype(m_sounds) cleaned;
            std::copy_if(m_sounds.begin(), m_sounds.end(), std::inserter(cleaned, cleaned.end()),
                         [](const std::weak_ptr<audio::SourceHandle>& h) {
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


        void ItemNode::update(const std::chrono::microseconds& deltaTime)
        {
            updateImpl(deltaTime, addTime(deltaTime));

            if( m_falling )
            {
                m_horizontalSpeed.add(getAccelleration(), deltaTime);
                if( getFallSpeed() >= 128 )
                {
                    m_fallSpeed.add(1, deltaTime);
                }
                else
                {
                    m_fallSpeed.add(6, deltaTime);
                }
            }
            else
            {
                m_horizontalSpeed = calculateFloorSpeed();
            }

            move(
                getMovementAngle().sin() * m_horizontalSpeed.getScaled(deltaTime),
                m_falling ? m_fallSpeed.getScaled(deltaTime) : 0,
                getMovementAngle().cos() * m_horizontalSpeed.getScaled(deltaTime)
                );

            applyTransform();

            updatePose();
            updateLighting();
        }


        boost::optional<uint16_t> ItemNode::getCurrentBox() const
        {
            auto sector = m_position.room->getInnerSectorByAbsolutePosition(m_position.position.toInexact());
            if( sector->boxIndex == 0xffff )
            {
                BOOST_LOG_TRIVIAL(warning) << "Whoops: " << getId();
                return {};
            }

            return sector->boxIndex;
        }
    }
}
