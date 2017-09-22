#include "itemnode.h"

#include "engine/laranode.h"
#include "level/level.h"


namespace engine
{
    namespace items
    {
        namespace
        {
            struct SpriteVertex
            {
                glm::vec3 pos;

                glm::vec2 uv;

                glm::vec3 color{ 1.0f };
            };

            std::shared_ptr<gameplay::Mesh> createSpriteMesh(const loader::Sprite& sprite, const std::shared_ptr<gameplay::Material>& material)
            {
                const SpriteVertex vertices[]{
                    { { sprite.left_side, sprite.top_side, 0 },{ sprite.t0.x, sprite.t1.y } },
                    { { sprite.right_side, sprite.top_side, 0 },{ sprite.t1.x, sprite.t1.y } },
                    { { sprite.right_side, sprite.bottom_side, 0 },{ sprite.t1.x, sprite.t0.y } },
                    { { sprite.left_side, sprite.bottom_side, 0 },{ sprite.t0.x, sprite.t0.y } }
                };

                gameplay::gl::StructuredVertexBuffer::AttributeMapping attribs{
                    { VERTEX_ATTRIBUTE_POSITION_NAME, gameplay::gl::VertexAttribute{ &SpriteVertex::pos } },
                    { VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gameplay::gl::VertexAttribute{ &SpriteVertex::uv } },
                    { VERTEX_ATTRIBUTE_COLOR_NAME, gameplay::gl::VertexAttribute{ &SpriteVertex::color } }
                };

                auto mesh = std::make_shared<gameplay::Mesh>(attribs, false);
                mesh->getBuffer(0)->assign<SpriteVertex>(vertices, 4);

                static const uint16_t indices[6] =
                {
                    0, 1, 2,
                    0, 2, 3
                };

                gameplay::gl::VertexArrayBuilder builder;

                auto indexBuffer = std::make_shared<gameplay::gl::IndexBuffer>();
                indexBuffer->setData(indices, 6, false);
                builder.attach(indexBuffer);
                builder.attach(mesh->getBuffers());

                auto part = std::make_shared<gameplay::MeshPart>(builder.build(material->getShaderProgram()->getHandle()));
                mesh->addPart(part);
                part->setMaterial(material);

                return mesh;
            }
        }

        void ItemNode::applyTransform()
        {
            glm::vec3 tr;

            if( const auto parent = m_position.room )
            {
                tr = m_position.position.toRenderSystem() - parent->position.toRenderSystem();
            }
            else
            {
                tr = m_position.position.toRenderSystem();
            }

            getNode()->setLocalMatrix(glm::translate(glm::mat4{1.0f}, tr) * getRotation().toMatrix());

            updateSounds();
        }


        ItemNode::ItemNode(const gsl::not_null<level::Level*>& level,
                           const gsl::not_null<const loader::Room*>& room,
                           const core::Angle& angle,
                           const core::TRCoordinates& position,
                           const floordata::ActivationState& activationState,
                           bool hasProcessAnimCommandsOverride,
                           Characteristics characteristics,
                           int16_t darkness)
            : m_position{room, position}
            , m_rotation{0_deg, angle, 0_deg}
            , m_level{level}
            , m_activationState{activationState}
            , m_hasProcessAnimCommandsOverride{hasProcessAnimCommandsOverride}
            , m_characteristics{characteristics}
            , m_darkness{darkness}
        {
            BOOST_ASSERT(room->isInnerPositionXZ(position));

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
            BOOST_LOG_TRIVIAL(debug) << "Room switch of " << getNode()->getId() << " to " << newRoom->node->getId();

            newRoom->node->addChild(getNode());

            m_position.room = newRoom;
        }


        ModelItemNode::ModelItemNode(const gsl::not_null<level::Level*>& level, const std::string& name,
                                     const gsl::not_null<const loader::Room*>& room, const core::Angle& angle,
                                     const core::TRCoordinates& position,
                                     const floordata::ActivationState& activationState,
                                     bool hasProcessAnimCommandsOverride, Characteristics characteristics,
                                     int16_t darkness,
                                     const loader::AnimatedModel& animatedModel)
            : ItemNode{
                level,
                room,
                angle,
                position,
                activationState,
                hasProcessAnimCommandsOverride,
                characteristics,
                darkness
            }
            , m_skeleton{std::make_shared<SkeletalModelNode>(name, level, animatedModel)}
        {
            m_skeleton->setAnimIdGlobal(animatedModel.animationIndex,
                                        getLevel().m_animations.at(animatedModel.animationIndex).firstFrame);
        }


        void ModelItemNode::update()
        {
            const auto endOfAnim = m_skeleton->advanceFrame();

            m_flags2_10_isHit = false;

            if( endOfAnim )
            {
                const loader::Animation& animation = m_skeleton->getCurrentAnimData();
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
                            setFallSpeed(cmd[0]);
                            setHorizontalSpeed(cmd[1]);
                            setFalling(true);
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

                const loader::Animation& currentAnim = m_skeleton->getCurrentAnimData();
                m_skeleton->setAnimIdGlobal(currentAnim.nextAnimation, currentAnim.nextFrame);
                m_skeleton->setTargetState(m_skeleton->getCurrentState());
            }

            const loader::Animation& animation = m_skeleton->getCurrentAnimData();
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
                        if( m_skeleton->getCurrentFrame() == cmd[0] )
                        {
                            playSoundEffect(cmd[1]);
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if( m_skeleton->getCurrentFrame() == cmd[0] )
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
                BOOST_LOG_TRIVIAL(trace) << "Activating item controller " << getNode()->getId();
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
                BOOST_LOG_TRIVIAL(trace) << "Deactivating item controller " << getNode()->getId();
            }

            m_isActive = false;
        }


        std::shared_ptr<audio::SourceHandle> ItemNode::playSoundEffect(int id)
        {
            auto handle = getLevel().playSound(id, getNode()->getTranslationWorld());
            if( handle != nullptr )
            {
                m_sounds.emplace(handle);
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
                lockedHandle->setPosition(getNode()->getTranslationWorld());
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

            const auto dist = glm::vec4{ (lara.getPosition() - item.getPosition()).toRenderSystem(), 1.0f } * item.getRotation().toMatrix();
            const glm::vec3 tdist{dist};

            return distance.contains(core::TRCoordinates{ tdist });
        }


        void ModelItemNode::applyMovement(bool forLara)
        {
            if( isFalling() )
            {
                if( getFallSpeed() >= 128 )
                {
                    setFallSpeed(getFallSpeed() + 1);
                }
                else
                {
                    setFallSpeed(getFallSpeed() + 6);
                }

                if( forLara )
                {
                    // we only add accelleration here
                    setHorizontalSpeed( getHorizontalSpeed() + m_skeleton->calculateFloorSpeed(0) - m_skeleton->calculateFloorSpeed(-1) );
                }
            }
            else
            {
                setHorizontalSpeed(m_skeleton->calculateFloorSpeed());
            }

            move(
                getMovementAngle().sin() * getHorizontalSpeed(),
                isFalling() ? getFallSpeed() : 0,
                getMovementAngle().cos() * getHorizontalSpeed()
            );

            applyTransform();

            m_skeleton->updatePose();
            updateLighting();
        }


        BoundingBox ModelItemNode::getBoundingBox() const
        {
            return m_skeleton->getBoundingBox();
        }


        boost::optional<uint16_t> ItemNode::getCurrentBox() const
        {
            const auto sector = m_position.room->getInnerSectorByAbsolutePosition(m_position.position);
            if( sector->boxIndex == 0xffff )
            {
                BOOST_LOG_TRIVIAL(warning) << "Not within a box: " << getNode()->getId();
                return {};
            }

            return sector->boxIndex;
        }


        uint16_t ModelItemNode::getCurrentState() const
        {
            return m_skeleton->getCurrentState();
        }


        SpriteItemNode::SpriteItemNode(const gsl::not_null<level::Level*>& level,
                                       const std::string& name,
                                       const gsl::not_null<const loader::Room*>& room,
                                       const core::Angle& angle,
                                       const core::TRCoordinates& position,
                                       const floordata::ActivationState& activationState,
                                       bool hasProcessAnimCommandsOverride,
                                       Characteristics characteristics,
                                       int16_t darkness,
                                       const loader::Sprite& sprite,
                                       const std::shared_ptr<gameplay::Material>& material,
                                       const std::vector<std::shared_ptr<gameplay::gl::Texture>>& textures)
            : ItemNode{
                level,
                room,
                angle,
                position,
                activationState,
                hasProcessAnimCommandsOverride,
                characteristics,
                darkness
            }
        {
            const auto spriteMesh = createSpriteMesh(sprite, material);
            auto model = std::make_shared<gameplay::Model>();
            model->addMesh(spriteMesh);

            m_node = std::make_shared<gameplay::Node>(name);
            m_node->setDrawable(model);
            m_node->addMaterialParameterSetter("u_diffuseTexture", [texture = textures[sprite.texture]](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform)
            {
                uniform.set(*texture);
            });
            m_node->addMaterialParameterSetter("u_baseLight", [darkness](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform)
            {
                uniform.set((8192 - darkness) / 32.0f);
            });
        }
    }
}
