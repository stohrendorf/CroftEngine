#include "itemnode.h"

#include "engine/laranode.h"
#include "level/level.h"

#include <boost/range/adaptor/indexed.hpp>

namespace engine
{
namespace items
{
namespace
{
const char* toString(const TriggerState s)
{
    switch( s )
    {
        case TriggerState::Inactive:
            return "Inactive";
        case TriggerState::Active:
            return "Active";
        case TriggerState::Deactivated:
            return "Deactivated";
        case TriggerState::Invisible:
            return "Invisible";
        default:
            BOOST_THROW_EXCEPTION( std::domain_error( "Invalid TriggerState" ) );
    }
}

TriggerState parseTriggerState(const std::string& s)
{
    if( s == "Inactive" )
        return TriggerState::Inactive;
    if( s == "Active" )
        return TriggerState::Active;
    if( s == "Deactivated" )
        return TriggerState::Deactivated;
    if( s == "Invisible" )
        return TriggerState::Invisible;
    BOOST_THROW_EXCEPTION( std::domain_error( "Invalid TriggerState" ) );
}
}

void ItemNode::applyTransform()
{
    const glm::vec3 tr = m_state.position.position.toRenderSystem() - m_state.position.room->position.toRenderSystem();
    getNode()->setLocalMatrix( translate( glm::mat4{1.0f}, tr ) * m_state.rotation.toMatrix() );
}

ItemNode::ItemNode(const gsl::not_null<level::Level*>& level,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const bool hasUpdateFunction)
        : m_level{level}
        , m_state{&level->m_soundEngine, room, item.type}
        , m_hasUpdateFunction{hasUpdateFunction}
{
    BOOST_ASSERT( room->isInnerPositionXZ( item.position ) );

    m_state.type = item.type;
    m_state.position.position = item.position;
    m_state.rotation.Y = core::Angle{item.rotation};
    m_state.shade = item.darkness;
    m_state.activationState = floordata::ActivationState( item.activationState );
    m_state.timer = m_state.activationState.getTimeout();

    m_state.floor = room->getSectorByAbsolutePosition( item.position )->floorHeight * loader::QuarterSectorSize;

    if( m_state.activationState.isOneshot() )
    {
        m_state.activationState.setOneshot( false );
        m_state.triggerState = TriggerState::Invisible;
    }

    if( m_state.activationState.isFullyActivated() )
    {
        m_state.activationState.fullyDeactivate();
        m_state.activationState.setInverted( true );
        activate();
        m_state.triggerState = TriggerState::Active;
    }
}

void ItemNode::setCurrentRoom(const gsl::not_null<const loader::Room*>& newRoom)
{
    if( newRoom == m_state.position.room )
    {
        return;
    }

    addChild( newRoom->node, getNode() );

    m_state.position.room = newRoom;
    applyTransform();
}

ModelItemNode::ModelItemNode(const gsl::not_null<level::Level*>& level,
                             const gsl::not_null<const loader::Room*>& room,
                             const loader::Item& item,
                             const bool hasUpdateFunction,
                             const loader::SkeletalModelType& animatedModel)
        : ItemNode{level, room, item, hasUpdateFunction}
        , m_skeleton{std::make_shared<SkeletalModelNode>(
                std::string( "skeleton(type:" ) + engine::toString( item.type ) + ")",
                level,
                animatedModel )
        }
{
    m_skeleton->setAnimation( m_state, animatedModel.animations, animatedModel.animations->firstFrame );

    for( gsl::index boneIndex = 0; boneIndex < animatedModel.meshes.size(); ++boneIndex )
    {
        auto node = std::make_shared<gameplay::Node>(
                m_skeleton->getId() + "/bone:" + std::to_string( boneIndex ) );
        node->setDrawable( animatedModel.models[boneIndex].get() );
        addChild( m_skeleton, node );
    }

    BOOST_ASSERT( m_skeleton->getChildren().size() == gsl::narrow<size_t>( animatedModel.meshes.size() ) );

    m_skeleton->updatePose( m_state );
}

void ModelItemNode::update()
{
    const auto endOfAnim = m_skeleton->advanceFrame( m_state );

    m_state.is_hit = false;
    m_state.touch_bits = 0;

    if( endOfAnim )
    {
        BOOST_ASSERT(
                m_state.anim->animCommandCount == 0
                || m_state.anim->animCommandIndex < getLevel().m_animCommands.size() );
        const auto* cmd = m_state.anim->animCommandCount == 0
                          ? nullptr
                          : &getLevel().m_animCommands[m_state.anim->animCommandIndex];
        for( uint16_t i = 0; i < m_state.anim->animCommandCount; ++i )
        {
            BOOST_ASSERT( cmd < &getLevel().m_animCommands.back() );
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
                    m_state.fallspeed = cmd[0];
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
                    m_state.triggerState = TriggerState::Deactivated;
                    break;
                default:
                    break;
            }
        }

        m_skeleton->setAnimation( m_state, m_state.anim->nextAnimation, m_state.anim->nextFrame );
        m_state.goal_anim_state = m_state.current_anim_state;
        if( m_state.current_anim_state == m_state.required_anim_state )
            m_state.required_anim_state = 0_as;
    }

    BOOST_ASSERT(
            m_state.anim->animCommandCount == 0 || m_state.anim->animCommandIndex < getLevel().m_animCommands.size() );
    const auto* cmd = m_state.anim->animCommandCount == 0
                      ? nullptr
                      : &getLevel().m_animCommands[m_state.anim->animCommandIndex];
    for( uint16_t i = 0; i < m_state.anim->animCommandCount; ++i )
    {
        BOOST_ASSERT( cmd < &getLevel().m_animCommands.back() );
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
                    playSoundEffect( static_cast<TR1SoundId>(cmd[1]) );
                }
                cmd += 2;
                break;
            case AnimCommandOpcode::PlayEffect:
                if( m_state.frame_number == cmd[0] )
                {
                    BOOST_LOG_TRIVIAL( debug ) << "Anim effect: " << int( cmd[1] );
                    getLevel().runEffect( cmd[1], this );
                }
                cmd += 2;
                break;
            default:
                break;
        }
    }

    applyMovement( false );
}

void ItemNode::activate()
{
    if( !m_hasUpdateFunction )
    {
        m_state.triggerState = TriggerState::Inactive;
        return;
    }

    m_isActive = true;
}

void ItemNode::deactivate()
{
    m_isActive = false;
}

std::shared_ptr<audio::SourceHandle> ItemNode::playSoundEffect(const TR1SoundId id)
{
    return getLevel().playSound( id, &m_state );
}

bool ItemNode::triggerKey()
{
    if( getLevel().m_lara->getHandStatus() != HandStatus::None )
    {
        return false;
    }

    if( m_state.triggerState != TriggerState::Active )
    {
        return false;
    }

    m_state.triggerState = TriggerState::Deactivated;
    return true;
}

void ItemNode::kill()
{
    if( this == getLevel().m_lara->target.get() )
    {
        getLevel().m_lara->target.reset();
    }
    getLevel().scheduleDeletion( this );
    m_state.activationState.setLocked( true );
}

bool ItemNode::triggerPickUp()
{
    if( m_state.triggerState != TriggerState::Invisible )
    {
        return false;
    }

    m_state.triggerState = TriggerState::Deactivated;
    return true;
}

YAML::Node ItemNode::save() const
{
    YAML::Node n;
    n["state"] = m_state.save( *m_level );
    n["active"] = m_isActive;

    for( const auto& child : getNode()->getChildren() )
        if( auto idx = m_level->indexOfModel( child->getDrawable() ) )
            n["meshes"].push_back( *idx );
        else
            n["meshes"].push_back( YAML::Node() );

    if( n["meshes"].IsDefined() )
        n["meshes"].SetStyle( YAML::EmitterStyle::Flow );

    return n;
}

void ItemNode::load(const YAML::Node& n)
{
    m_state.load( n["state"], *m_level );
    m_isActive = n["active"].as<bool>();

    if( getNode()->getChildren().empty() )
        Expects( !n["meshes"].IsDefined() );
    else
        Expects( !n["meshes"].IsDefined() || n["meshes"].size() <= getNode()->getChildren().size() );

    if( n["meshes"].IsDefined() )
    {
        for( size_t i = 0; i < n["meshes"].size(); ++i )
        {
            auto m = n["meshes"][i];
            if( !m.IsNull() )
                getNode()->getChildren()[i]->setDrawable( getLevel().getModel( m.as<size_t>() ).get() );
            else
                getNode()->getChildren()[i]->setDrawable( nullptr );
        }
    }

    addChild( m_state.position.room->node, getNode() );
    applyTransform();
}

bool InteractionLimits::canInteract(const ItemState& item, const ItemState& lara) const
{
    const auto angle = lara.rotation - item.rotation;
    if( angle.X < minAngle.X || angle.X > maxAngle.X
        || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
        || angle.Z < minAngle.Z || angle.Z > maxAngle.Z )
    {
        return false;
    }

    const auto offs = lara.position.position - item.position.position;
    const auto dist = glm::vec4{offs.toRenderSystem(), 0.0f} * item.rotation.toMatrix();
    return distance.contains( core::TRVec{glm::vec3{dist}} );
}

void ModelItemNode::applyMovement(const bool forLara)
{
    if( m_state.falling )
    {
        if( m_state.fallspeed >= 128 )
        {
            m_state.fallspeed += 1;
        }
        else
        {
            m_state.fallspeed += 6;
        }

        if( forLara )
        {
            // we only add acceleration here
            m_state.speed = m_state.speed + m_skeleton->calculateFloorSpeed( m_state, 0 )
                            - m_skeleton->calculateFloorSpeed( m_state, -1 );
        }
    }
    else
    {
        m_state.speed = m_skeleton->calculateFloorSpeed( m_state );
    }

    move(
            getMovementAngle().sin() * m_state.speed,
            m_state.falling ? m_state.fallspeed : 0,
            getMovementAngle().cos() * m_state.speed
    );

    applyTransform();

    m_skeleton->updatePose( m_state );
    updateLighting();
}

loader::BoundingBox ModelItemNode::getBoundingBox() const
{
    return m_skeleton->getBoundingBox( m_state );
}

SpriteItemNode::SpriteItemNode(const gsl::not_null<level::Level*>& level,
                               const std::string& name,
                               const gsl::not_null<const loader::Room*>& room,
                               const loader::Item& item,
                               const bool hasUpdateFunction,
                               const loader::Sprite& sprite,
                               const gsl::not_null<std::shared_ptr<gameplay::Material>>& material)
        : ItemNode{level, room, item, hasUpdateFunction}
{
    const auto model = std::make_shared<gameplay::Sprite>( sprite.x0, -sprite.y0,
                                                           sprite.x1, -sprite.y1,
                                                           sprite.t0, sprite.t1,
                                                           material,
                                                           gameplay::Sprite::Axis::Y );

    m_node = std::make_shared<gameplay::Node>( name );
    m_node->setDrawable( model );
    m_node->addMaterialParameterSetter( "u_diffuseTexture",
                                        [texture = sprite.texture](const gameplay::Node& /*node*/,
                                                                   gameplay::gl::Program::ActiveUniform& uniform) {
                                            uniform.set( *texture );
                                        } );
    m_node->addMaterialParameterSetter( "u_baseLight",
                                        [brightness = item.getBrightness()](const gameplay::Node& /*node*/,
                                                                            gameplay::gl::Program::ActiveUniform& uniform) {
                                            uniform.set( brightness );
                                        } );
    m_node->addMaterialParameterSetter( "u_baseLightDiff",
                                        [](const gameplay::Node& /*node*/,
                                           gameplay::gl::Program::ActiveUniform& uniform) {
                                            uniform.set( 0.0f );
                                        } );
    m_node->addMaterialParameterSetter( "u_lightPosition",
                                        [](const gameplay::Node& /*node*/,
                                           gameplay::gl::Program::ActiveUniform& uniform) {
                                            uniform.set( glm::vec3{std::numeric_limits<float>::quiet_NaN()} );
                                        } );

    addChild( room->node, m_node );

    applyTransform();
}

bool ModelItemNode::isNear(const ModelItemNode& other, const int radius) const
{
    const auto aFrame = getSkeleton()->getInterpolationInfo( m_state ).getNearestFrame();
    const auto bFrame = other.getSkeleton()->getInterpolationInfo( other.m_state ).getNearestFrame();
    if( other.m_state.position.position.Y + bFrame->bbox.minY >= m_state.position.position.Y + aFrame->bbox.maxY
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

bool ModelItemNode::isNear(const Particle& other, const int radius) const
{
    const auto aFrame = getSkeleton()->getInterpolationInfo( m_state ).getNearestFrame();
    if( other.pos.position.Y >= m_state.position.position.Y + aFrame->bbox.maxY
        || m_state.position.position.Y + aFrame->bbox.minY >= other.pos.position.Y )
    {
        return false;
    }

    const auto c = m_state.rotation.Y.cos();
    const auto s = m_state.rotation.Y.sin();
    const auto dx = other.pos.position.X - m_state.position.position.X;
    const auto dz = other.pos.position.Z - m_state.position.position.Z;
    const auto x = c * dx - s * dz;
    const auto z = s * dx + c * dz;
    return x >= aFrame->bbox.minX - radius
           && x <= aFrame->bbox.maxX + radius
           && z >= aFrame->bbox.minZ - radius
           && z <= aFrame->bbox.maxZ + radius;
}

void ModelItemNode::enemyPush(LaraNode& lara, CollisionInfo& collisionInfo, const bool enableSpaz,
                              const bool withXZCollRadius)
{
    const auto dx = lara.m_state.position.position.X - m_state.position.position.X;
    const auto dz = lara.m_state.position.position.Z - m_state.position.position.Z;
    const auto c = m_state.rotation.Y.cos();
    const auto s = m_state.rotation.Y.sin();
    auto posX = c * dx - s * dz;
    auto posZ = s * dx + c * dz;
    const auto itemKeyFrame = m_skeleton->getInterpolationInfo( m_state ).getNearestFrame();
    auto itemBBoxMinX = itemKeyFrame->bbox.minX;
    auto itemBBoxMaxX = itemKeyFrame->bbox.maxX;
    auto itemBBoxMaxZ = itemKeyFrame->bbox.maxZ;
    auto itemBBoxMinZ = itemKeyFrame->bbox.minZ;
    if( withXZCollRadius )
    {
        const auto r = collisionInfo.collisionRadius;
        itemBBoxMinX -= r;
        itemBBoxMaxX += r;
        itemBBoxMinZ -= r;
        itemBBoxMaxZ += r;
    }
    if( posX >= itemBBoxMinX && posX <= itemBBoxMaxX && posZ >= itemBBoxMinZ && posZ <= itemBBoxMaxZ )
    {
        const auto dxMin = posX - itemBBoxMinX;
        const auto dxMax = itemBBoxMaxX - posX;
        const auto dzMin = posZ - itemBBoxMinZ;
        const auto dzMax = itemBBoxMaxZ - posZ;
        if( posX - itemBBoxMinX <= itemBBoxMaxX - posX && dxMin <= dzMax && dxMin <= dzMin )
        {
            posX -= dxMin;
        }
        else if( dxMin >= dxMax && dxMax <= dzMax && dxMax <= dzMin )
        {
            posX = itemBBoxMaxX;
        }
        else if( dxMin < dzMax || dzMax > dxMax || dzMax > dzMin )
        {
            posZ -= dzMin;
        }
        else
        {
            posZ = itemBBoxMaxZ;
        }
        lara.m_state.position.position.X = ((c * posX + s * posZ)) + m_state.position.position.X;
        lara.m_state.position.position.Z = ((c * posZ - s * posX)) + m_state.position.position.Z;
        if( enableSpaz )
        {
            const auto midX = (itemKeyFrame->bbox.minX + itemKeyFrame->bbox.maxX) / 2;
            const auto midZ = (itemKeyFrame->bbox.minZ + itemKeyFrame->bbox.maxZ) / 2;
            const auto a = core::Angle::fromAtan( dx - ((midX * c + midZ * s)), dz - ((midZ * c - midX * s)) )
                           - 180_deg;
            getLevel().m_lara->hit_direction = axisFromAngle( lara.m_state.rotation.Y - a, 45_deg ).get();
            if( getLevel().m_lara->hit_frame == 0 )
            {
                lara.playSoundEffect( TR1SoundId::LaraOof );
            }
            if( ++getLevel().m_lara->hit_frame > 34 )
            {
                getLevel().m_lara->hit_frame = 34;
            }
        }
        collisionInfo.badPositiveDistance = loader::HeightLimit;
        collisionInfo.badNegativeDistance = -384;
        collisionInfo.badCeilingDistance = 0;
        const auto facingAngle = collisionInfo.facingAngle;
        collisionInfo.facingAngle = core::Angle::fromAtan(
                lara.m_state.position.position.X - collisionInfo.oldPosition.X,
                lara.m_state.position.position.Z - collisionInfo.oldPosition.Z );
        collisionInfo.initHeightInfo( lara.m_state.position.position, getLevel(), 762 );
        collisionInfo.facingAngle = facingAngle;
        if( collisionInfo.collisionType != CollisionInfo::AxisColl::None )
        {
            lara.m_state.position.position.X = collisionInfo.oldPosition.X;
            lara.m_state.position.position.Z = collisionInfo.oldPosition.Z;
        }
        else
        {
            collisionInfo.oldPosition = lara.m_state.position.position;
            lara.updateFloorHeight( -10 );
        }
    }
}

bool ModelItemNode::testBoneCollision(const ModelItemNode& other)
{
    m_state.touch_bits = 0;
    const auto itemSpheres = m_skeleton->getBoneCollisionSpheres(
            m_state,
            *m_skeleton->getInterpolationInfo( m_state ).getNearestFrame(),
            nullptr );
    const auto laraSpheres = other.m_skeleton->getBoneCollisionSpheres(
            other.m_state,
            *other.m_skeleton->getInterpolationInfo( other.m_state ).getNearestFrame(),
            nullptr );
    for( const auto& itemSphere : itemSpheres | boost::adaptors::indexed( 0 ) )
    {
        if( itemSphere.value().radius <= 0 )
            continue;

        for( const auto& laraSphere : laraSpheres )
        {
            if( laraSphere.radius <= 0 )
                continue;
            if( distance( laraSphere.getPosition(), itemSphere.value().getPosition() )
                >= util::square( itemSphere.value().radius + laraSphere.radius ) )
                continue;

            m_state.touch_bits |= 1 << itemSphere.index();
            break;
        }
    }

    return m_state.touch_bits != 0;
}

gsl::not_null<std::shared_ptr<Particle>> ModelItemNode::emitParticle(const core::TRVec& localPosition,
                                                                     const size_t boneIndex,
                                                                     gsl::not_null<std::shared_ptr<Particle>> (* generate)(
                                                                             level::Level& level,
                                                                             const core::RoomBoundPosition&,
                                                                             int16_t, core::Angle))
{
    BOOST_ASSERT( generate != nullptr );
    BOOST_ASSERT( boneIndex < m_skeleton->getChildren().size() );

    const auto itemSpheres = m_skeleton->getBoneCollisionSpheres(
            m_state,
            *m_skeleton->getInterpolationInfo( m_state ).getNearestFrame(),
            nullptr );
    BOOST_ASSERT( boneIndex < itemSpheres.size() );

    auto roomPos = m_state.position;
    roomPos.position = core::TRVec{
            glm::vec3{translate( itemSpheres.at( boneIndex ).m, localPosition.toRenderSystem() )[3]}};
    auto particle = generate( getLevel(), roomPos, m_state.speed, m_state.rotation.Y );
    getLevel().m_particles.emplace_back( particle );

    return particle;
}

void ModelItemNode::load(const YAML::Node& n)
{
    ItemNode::load( n );

    m_skeleton->load( n["skeleton"] );
    m_skeleton->updatePose( m_state );
}

YAML::Node ModelItemNode::save() const
{
    auto n = ItemNode::save();
    n["skeleton"] = m_skeleton->save();
    return n;
}

bool ItemState::stalkBox(const level::Level& lvl, const loader::Box& box) const
{
    const auto laraToBoxDistX = (box.xmin + box.xmax) / 2 - lvl.m_lara->m_state.position.position.X;
    const auto laraToBoxDistZ = (box.zmin + box.zmax) / 2 - lvl.m_lara->m_state.position.position.Z;

    if( std::abs( laraToBoxDistX ) > 3 * loader::SectorSize || std::abs( laraToBoxDistZ ) > 3 * loader::SectorSize )
    {
        return false;
    }

    const auto laraAxisBack = *axisFromAngle( lvl.m_lara->m_state.rotation.Y + 180_deg, 45_deg );
    core::Axis laraToBoxAxis;
    if( laraToBoxDistZ > 0 )
    {
        if( laraToBoxDistX > 0 )
        {
            laraToBoxAxis = core::Axis::PosX;
        }
        else
        {
            laraToBoxAxis = core::Axis::NegZ;
        }
    }
    else if( laraToBoxDistX > 0 )
    {
        // Z <= 0, X > 0
        laraToBoxAxis = core::Axis::NegX;
    }
    else
    {
        // Z <= 0, X <= 0
        laraToBoxAxis = core::Axis::PosZ;
    }

    if( laraAxisBack == laraToBoxAxis )
    {
        return false;
    }

    core::Axis itemToLaraAxis;
    if( position.position.Z <= lvl.m_lara->m_state.position.position.Z )
    {
        if( position.position.X <= lvl.m_lara->m_state.position.position.X )
        {
            itemToLaraAxis = core::Axis::PosZ;
        }
        else
        {
            itemToLaraAxis = core::Axis::NegX;
        }
    }
    else
    {
        if( position.position.X > lvl.m_lara->m_state.position.position.X )
        {
            itemToLaraAxis = core::Axis::PosX;
        }
        else
        {
            itemToLaraAxis = core::Axis::NegZ;
        }
    }

    if( laraAxisBack != itemToLaraAxis )
    {
        return true;
    }

    switch( laraAxisBack )
    {
        case core::Axis::PosZ:
            return laraToBoxAxis == core::Axis::NegZ;
        case core::Axis::PosX:
            return laraToBoxAxis == core::Axis::NegX;
        case core::Axis::NegZ:
            return laraToBoxAxis == core::Axis::PosZ;
        case core::Axis::NegX:
            return laraToBoxAxis == core::Axis::PosX;
    }

    BOOST_THROW_EXCEPTION( std::runtime_error( "Unreachable code reached" ) );
}

bool ItemState::isInsideZoneButNotInBox(const level::Level& lvl,
                                        const int16_t zoneId,
                                        const loader::Box& box) const
{
    Expects( creatureInfo != nullptr );

    const auto zoneRef = loader::Box::getZoneRef( lvl.roomsAreSwapped, creatureInfo->lot.fly,
                                                  creatureInfo->lot.step );

    if( zoneId != box.*zoneRef )
    {
        return false;
    }

    if( (creatureInfo->lot.block_mask & box.overlap_index) != 0 )
    {
        return false;
    }

    return position.position.Z <= box.zmin
           || position.position.Z >= box.zmax
           || position.position.X <= box.xmin
           || position.position.X >= box.xmax;

}

bool ItemState::inSameQuadrantAsBoxRelativeToLara(const level::Level& lvl, const loader::Box& box) const
{
    const auto laraToBoxX = (box.xmin + box.xmax) / 2 - lvl.m_lara->m_state.position.position.X;
    const auto laraToBoxZ = (box.zmin + box.zmax) / 2 - lvl.m_lara->m_state.position.position.Z;
    if( std::abs( laraToBoxX ) < 5 * loader::SectorSize && std::abs( laraToBoxZ ) < 5 * loader::SectorSize )
        return false;

    const auto laraToNpcX = position.position.X - lvl.m_lara->m_state.position.position.X;
    const auto laraToNpcZ = position.position.Z - lvl.m_lara->m_state.position.position.Z;
    return ((laraToNpcZ > 0) == (laraToBoxZ > 0)) || ((laraToNpcX > 0) == (laraToBoxX > 0));

}

void ItemState::initCreatureInfo(const level::Level& lvl)
{
    if( creatureInfo != nullptr )
        return;

    creatureInfo = std::make_shared<ai::CreatureInfo>( lvl, this );
    collectZoneBoxes( lvl );
}

void ItemState::collectZoneBoxes(const level::Level& lvl)
{
    const auto zoneRef1 = loader::Box::getZoneRef( false, creatureInfo->lot.fly, creatureInfo->lot.step );
    const auto zoneRef2 = loader::Box::getZoneRef( true, creatureInfo->lot.fly, creatureInfo->lot.step );

    box = position.room->getInnerSectorByAbsolutePosition( position.position )->box;
    const auto zoneData1 = box->*zoneRef1;
    const auto zoneData2 = box->*zoneRef2;
    creatureInfo->lot.boxes.clear();
    for( const auto& box : lvl.m_boxes )
    {
        if( box.*zoneRef1 == zoneData1 || box.*zoneRef2 == zoneData2 )
        {
            creatureInfo->lot.boxes.emplace_back( &box );
        }
    }
}

sol::usertype<ItemState>& ItemState::userType()
{
    static auto type = sol::usertype<ItemState>(
            sol::meta_function::construct, sol::no_constructor,
            "position", [](ItemState& self) { return self.position.position; },
            "rotation", [](ItemState& self) { return self.rotation; },
            "current_anim_state", sol::readonly( &ItemState::current_anim_state ),
            "goal_anim_state", &ItemState::goal_anim_state,
            "required_anim_state", &ItemState::required_anim_state,
            "activation_state", &ItemState::triggerState,
            "patch_heights", [](ItemState& /*self*/, int /*delta*/) { /* TODO */ },
            "health", &ItemState::health,
            "do_enemy_push", []() { /* TODO */ },
            "set_y_angle", [](ItemState& self, const int16_t angle) { self.rotation.Y = core::Angle( angle ); },
            "set_collidable", [](ItemState& self, const bool flag) { self.collidable = flag; },
            "frame_number", &ItemState::frame_number,
            "enable_ai", &ItemState::initCreatureInfo,
            "creature_info", sol::readonly( &ItemState::creatureInfo )
    );
    return type;
}

YAML::Node ItemState::save(const level::Level& lvl) const
{
    YAML::Node n;
    n["type"] = engine::toString( type );
    n["position"] = position.position.save();
    n["position"]["room"] = std::distance( &lvl.m_rooms[0], position.room.get() );
    n["rotation"] = rotation.save();
    n["speed"] = speed;
    n["fallSpeed"] = fallspeed;
    n["state"] = current_anim_state.get();
    n["goal"] = goal_anim_state.get();
    n["required"] = required_anim_state.get();
    if( anim != nullptr )
        n["id"] = std::distance( &lvl.m_animations[0], anim );
    n["frame"] = frame_number;
    n["health"] = health;
    n["triggerState"] = toString( triggerState );
    n["timer"] = timer;
    n["activationState"] = activationState.save();
    n["floor"] = floor;
    n["touchBits"] = touch_bits.to_ulong();
    if( box != nullptr )
        n["box"] = std::distance( &lvl.m_boxes[0], box );
    n["shade"] = shade;

    n["falling"] = falling;
    n["isHit"] = is_hit;
    n["collidable"] = collidable;
    n["alreadyLookedAt"] = already_looked_at;

    if( creatureInfo != nullptr )
        n["creatureInfo"] = creatureInfo->save( lvl );

    return n;
}

void ItemState::load(const YAML::Node& n, const level::Level& lvl)
{
    if( EnumUtil<TR1ItemId>::fromString( n["type"].as<std::string>() ) != type )
        BOOST_THROW_EXCEPTION( std::domain_error( "Item state has wrong type" ) );

    position.position.load( n["position"] );
    position.room = &lvl.m_rooms.at( n["position"]["room"].as<size_t>() );
    rotation.load( n["rotation"] );
    speed = n["speed"].as<int16_t>();
    fallspeed = n["fallSpeed"].as<int16_t>();
    current_anim_state = loader::AnimState{n["state"].as<uint16_t>()};
    goal_anim_state = loader::AnimState{n["goal"].as<uint16_t>()};
    required_anim_state = loader::AnimState{n["required"].as<uint16_t>()};
    if( !n["id"].IsDefined() )
        anim = nullptr;
    else
        anim = &lvl.m_animations.at( n["id"].as<size_t>() );
    frame_number = n["frame"].as<uint16_t>();
    health = n["health"].as<int16_t>();
    triggerState = parseTriggerState( n["triggerState"].as<std::string>() );
    timer = n["timer"].as<int16_t>();
    activationState.load( n["activationState"] );

    floor = n["floor"].as<int32_t>();
    touch_bits = n["touchBits"].as<uint32_t>();
    if( !n["box"].IsDefined() )
        box = nullptr;
    else
        box = &lvl.m_boxes.at( n["box"].as<size_t>() );
    shade = n["shade"].as<int16_t>();

    falling = n["falling"].as<bool>();
    is_hit = n["isHit"].as<bool>();
    collidable = n["collidable"].as<bool>();
    already_looked_at = n["alreadyLookedAt"].as<bool>();

    if( !n["creatureInfo"].IsDefined() )
    {
        creatureInfo = nullptr;
    }
    else
    {
        creatureInfo = std::make_shared<ai::CreatureInfo>( lvl, this );
        creatureInfo->load( n["creatureInfo"], lvl );
    }
}

glm::vec3 ItemState::getPosition() const
{
    return position.position.toRenderSystem();
}

ItemState::~ItemState() = default;

void ItemNode::playShotMissed(const core::RoomBoundPosition& pos)
{
    const auto particle = std::make_shared<RicochetParticle>( pos, getLevel() );
    setParent( particle, m_state.position.room->node );
    getLevel().m_particles.emplace_back( particle );
    getLevel().playSound( TR1SoundId::Ricochet, particle.get() );
}

boost::optional<int> ItemNode::getWaterSurfaceHeight() const
{
    return m_state.position.room->getWaterSurfaceHeight( m_state.position );
}
}
}
