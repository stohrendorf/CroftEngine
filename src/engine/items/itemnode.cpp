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

    glm::vec3 color{1.0f};
};

std::shared_ptr<gameplay::Mesh> createSpriteMesh(const loader::Sprite& sprite, const std::shared_ptr<gameplay::Material>& material)
{
    const SpriteVertex vertices[]{
        {{sprite.left_side,  sprite.top_side,    0}, {sprite.t0.x, sprite.t1.y}},
        {{sprite.right_side, sprite.top_side,    0}, {sprite.t1.x, sprite.t1.y}},
        {{sprite.right_side, sprite.bottom_side, 0}, {sprite.t1.x, sprite.t0.y}},
        {{sprite.left_side,  sprite.bottom_side, 0}, {sprite.t0.x, sprite.t0.y}}
    };

    gameplay::gl::StructuredVertexBuffer::AttributeMapping attribs{
        {VERTEX_ATTRIBUTE_POSITION_NAME,        gameplay::gl::VertexAttribute{&SpriteVertex::pos}},
        {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gameplay::gl::VertexAttribute{&SpriteVertex::uv}},
        {VERTEX_ATTRIBUTE_COLOR_NAME,           gameplay::gl::VertexAttribute{&SpriteVertex::color}}
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

    if( const auto parent = m_state.position.room )
    {
        tr = m_state.position.position.toRenderSystem() - parent->position.toRenderSystem();
    }
    else
    {
        tr = m_state.position.position.toRenderSystem();
    }

    getNode()->setLocalMatrix(glm::translate(glm::mat4{1.0f}, tr) * m_state.rotation.toMatrix());

    updateSounds();
}

ItemNode::ItemNode(const gsl::not_null<level::Level*>& level,
                   const gsl::not_null<const loader::Room*>& room,
                   const core::Angle& angle,
                   const core::TRCoordinates& position,
                   uint16_t activationState,
                   bool hasProcessAnimCommandsOverride,
                   Characteristics characteristics,
                   int16_t darkness)
    : m_level{level}
      , m_hasProcessAnimCommandsOverride{hasProcessAnimCommandsOverride}
      , m_characteristics{characteristics}
{
    BOOST_ASSERT(room->isInnerPositionXZ(position));

    m_state.position.position = position;
    m_state.position.room = room;
    m_state.rotation.Y = angle;
    m_state.shade = darkness;
    m_state.activationState = engine::floordata::ActivationState(activationState);
    m_state.timer = engine::floordata::ActivationState::extractTimeout(activationState);

    if( m_state.activationState.isOneshot() )
    {
        m_state.activationState.setOneshot(false);
        m_state.triggerState = TriggerState::Locked;
    }

    if( m_state.activationState.isFullyActivated() )
    {
        m_state.activationState.fullyDeactivate();
        m_state.activationState.setInverted(true);
        activate();
        m_state.triggerState = TriggerState::Enabled;
    }
}

void ItemNode::setCurrentRoom(const loader::Room* newRoom)
{
    if( newRoom == m_state.position.room )
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

    m_state.position.room = newRoom;
}

ModelItemNode::ModelItemNode(const gsl::not_null<level::Level*>& level, const std::string& name,
                             const gsl::not_null<const loader::Room*>& room, const core::Angle& angle,
                             const core::TRCoordinates& position,
                             uint16_t activationState,
                             bool hasProcessAnimCommandsOverride, Characteristics characteristics,
                             int16_t darkness,
                             const loader::SkeletalModelType& animatedModel)
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
    m_skeleton->setAnimIdGlobal(m_state,
                                animatedModel.animationIndex,
                                getLevel().m_animations.at(animatedModel.animationIndex).firstFrame);
}

void ModelItemNode::update()
{
    const auto endOfAnim = m_skeleton->advanceFrame(m_state);

    m_state.is_hit = false;

    if( endOfAnim )
    {
        const loader::Animation& animation = m_skeleton->getCurrentAnimData(m_state);
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
                {
                    m_state.fallspeed = cmd[0];
                }
                    m_state.speed = cmd[1];
                    m_state.falling = true;
                    cmd += 2;
                    break;
                case AnimCommandOpcode::PlaySound:
                    cmd += 2;
                    break;
                case AnimCommandOpcode::PlayEffect:
                    cmd += 2;
                    break;
                case AnimCommandOpcode::Kill:
                    m_state.triggerState = TriggerState::Activated;
                    break;
                default:
                    break;
            }
        }

        const loader::Animation& currentAnim = m_skeleton->getCurrentAnimData(m_state);
        m_skeleton->setAnimIdGlobal(m_state, currentAnim.nextAnimation, currentAnim.nextFrame);
        m_state.goal_anim_state = m_state.current_anim_state;
    }

    const loader::Animation& animation = m_skeleton->getCurrentAnimData(m_state);
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
                if( m_state.frame_number == cmd[0] )
                {
                    playSoundEffect(cmd[1]);
                }
                cmd += 2;
                break;
            case AnimCommandOpcode::PlayEffect:
                if( m_state.frame_number == cmd[0] )
                {
                    BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                    if( cmd[1] == 0 )
                    {
                        m_state.rotation.Y += 180_deg;
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
        m_state.triggerState = TriggerState::Disabled;
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

    if( m_state.triggerState != TriggerState::Enabled )
    {
        return false;
    }

    m_state.triggerState = TriggerState::Activated;
    return true;
}

void ItemNode::updateSounds()
{
    decltype(m_sounds) cleaned;
    std::copy_if(m_sounds.begin(), m_sounds.end(), inserter(cleaned, cleaned.end()),
                 [](const std::weak_ptr<audio::SourceHandle>& h) {
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
    const auto angle = lara.m_state.rotation - item.m_state.rotation;
    if( angle.X < minAngle.X || angle.X > maxAngle.X
        || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
        || angle.Z < minAngle.Z || angle.Z > maxAngle.Z )
    {
        return false;
    }

    const auto dist = glm::vec4{(lara.m_state.position.position - item.m_state.position.position).toRenderSystem(), 1.0f} * item.m_state.rotation.toMatrix();
    const glm::vec3 tdist{dist};

    return distance.contains(core::TRCoordinates{tdist});
}

void ModelItemNode::applyMovement(bool forLara)
{
    if( m_state.falling )
    {
        if( m_state.fallspeed >= 128 )
        {
            const int16_t spd = m_state.fallspeed + 1;
            m_state.fallspeed = spd;
        }
        else
        {
            const int16_t spd = m_state.fallspeed + 6;
            m_state.fallspeed = spd;
        }

        if( forLara )
        {
            // we only add accelleration here
            const int16_t speed = m_state.speed + m_skeleton->calculateFloorSpeed(m_state, 0) - m_skeleton->calculateFloorSpeed(m_state, -1);
            m_state.speed = speed;
        }
    }
    else
    {
        const int16_t speed = m_skeleton->calculateFloorSpeed(m_state);
        m_state.speed = speed;
    }

    move(
        getMovementAngle().sin() * m_state.speed,
        m_state.falling ? m_state.fallspeed : 0,
        getMovementAngle().cos() * m_state.speed
        );

    applyTransform();

    m_skeleton->updatePose(m_state);
    updateLighting();
}

BoundingBox ModelItemNode::getBoundingBox() const
{
    return m_skeleton->getBoundingBox(m_state);
}

boost::optional<uint16_t> ItemNode::getCurrentBox() const
{
    const auto sector = m_state.position.room->getInnerSectorByAbsolutePosition(m_state.position.position);
    if( sector->boxIndex == 0xffff )
    {
        BOOST_LOG_TRIVIAL(warning) << "Not within a box: " << getNode()->getId();
        return {};
    }

    return sector->boxIndex;
}

uint16_t ModelItemNode::getCurrentState() const
{
    return m_state.current_anim_state;
}

SpriteItemNode::SpriteItemNode(const gsl::not_null<level::Level*>& level,
                               const std::string& name,
                               const gsl::not_null<const loader::Room*>& room,
                               const core::Angle& angle,
                               const core::TRCoordinates& position,
                               uint16_t activationState,
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
    m_node->addMaterialParameterSetter("u_diffuseTexture",
                                       [texture = textures[sprite.texture]](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
                                           uniform.set(*texture);
                                       });
    m_node->addMaterialParameterSetter("u_baseLight", [darkness](const gameplay::Node& /*node*/, gameplay::gl::Program::ActiveUniform& uniform) {
        uniform.set((8192 - darkness) / 32.0f);
    });
}

bool ModelItemNode::isNear(const ModelItemNode& other, const int radius) const
{
    const auto aFrame = getSkeleton()->getInterpolationInfo(m_state).getNearestFrame();
    const auto bFrame = other.getSkeleton()->getInterpolationInfo(m_state).getNearestFrame();
    if( other.m_state.position.position.Y + bFrame->bbox.minY >= aFrame->bbox.maxY + m_state.position.position.Y
        || m_state.position.position.Y + aFrame->bbox.minY >= other.m_state.position.position.Y + bFrame->bbox.maxY )
    {
        return false;
    }

    const auto c = m_state.rotation.Y.cos();
    const auto s = m_state.rotation.Y.sin();
    const auto dx = other.m_state.position.position.X - m_state.position.position.X;
    const auto dz = other.m_state.position.position.Z - m_state.position.position.Z;
    const auto x = c * dx - s * dz;
    const auto z = s * dx + c * dz;
    return x >= aFrame->bbox.minX - radius
           && x <= aFrame->bbox.maxX + radius
           && z >= aFrame->bbox.minZ - radius
           && z <= aFrame->bbox.maxZ + radius;
}
}
}
